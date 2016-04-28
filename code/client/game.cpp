#include <stdio.h>
#include <math.h>
#include "lib/def.h"
#include "lib/assert.h"
#include "lib/int_seq.h"
#include "lib/chunk_ring_buffer.h"
#include "common/memory.h"
#include "common/net_messages.h"
#include "common/simulation.h"
#include "common/order_serialization.h"
#include "coors.h"
#include "interpolation.h"
#include "net_events.h"
#include "net_commands.h"
#include "render_commands.h"
#include "game.h"

static const ui32 Red = 0x00FF0000;
static const ui32 Blue = 0x000000FF;
static const ui32 White = 0x00FFFFFF;
static const r32 Zoom = 1.0 / 1000.0;

static const ui32 PlayerColors[] = {
  Red, Blue
};

#define UNIT_SELECTION_MAX 8;
struct unit_selection {
  simulation_unit_id IDs[8];
  ui8 Count;
};

enum game_mode {
  game_mode_waiting,
  game_mode_active
};

struct game_state {
  linear_allocator Allocator;
  buffer CommandSerializationBuffer;
  simulation_player_id PlayerID;
  simulation Sim;
  interpolation Interpolation;
  memsize *OrderListCounts;
  int_seq OrderListCountSeq;
  uusec64 NextTickTime;
  uusec64 NextExtraTickTime;
  chunk_ring_buffer OrderListRing;
  unit_selection UnitSelection;
  game_mode Mode;
};

static r32 GetAspectRatio(ivec2 Resolution) {
  rvec2 Real = ConvertIvec2ToRvec2(Resolution);
  return Real.X / Real.Y;
}

void InitGame(buffer Memory) {
  game_state *State = (game_state*)Memory.Addr;
  {
    void *Base = (ui8*)Memory.Addr + sizeof(game_state);
    memsize Length = Memory.Length - sizeof(game_state);
    InitLinearAllocator(&State->Allocator, Base, Length);
  }

  {
    buffer *B = &State->CommandSerializationBuffer;
    B->Addr = LinearAllocate(&State->Allocator, NETWORK_COMMAND_MAX_LENGTH);
    B->Length = NETWORK_COMMAND_MAX_LENGTH;
  }

  {
    usec32 WatchDuration = 10;
    memsize SamplesPerSecond = 1000 / SimulationTickDuration;
    memsize SequenceLength = SamplesPerSecond * WatchDuration;
    State->OrderListCounts = (memsize*)LinearAllocate(&State->Allocator, sizeof(memsize)*SequenceLength);
    InitIntSeq(&State->OrderListCountSeq, State->OrderListCounts, SequenceLength);
  }

  {
    memsize Count = 100;
    memsize StorageSize = Count * 1024;
    void *Storage = LinearAllocate(&State->Allocator, StorageSize);
    buffer Buffer = {
      .Addr = Storage,
      .Length = StorageSize
    };
    InitChunkRingBuffer(&State->OrderListRing, Count, Buffer);
  }

  State->PlayerID = SIMULATION_UNDEFINED_PLAYER_ID;
  State->UnitSelection.Count = 0;
  State->Mode = game_mode_waiting;
}

#define AddRenderCommand(List, Type) (Type##_render_command*)_AddRenderCommand(List, render_command_type_##Type, sizeof(Type##_render_command))

void* _AddRenderCommand(chunk_list *List, render_command_type Type, memsize Length) {
  Length += sizeof(Type);
  void *Base = ChunkListAllocate(List, Length);
  render_command_type *TypePtr = (render_command_type*)Base;
  *TypePtr = Type;
  ui8* Command = (ui8*)Base + sizeof(Type);
  return (void*)Command;
}

void Render(simulation *Sim, interpolation *Interpolation, unit_selection *UnitSelection, chunk_list *Commands, ivec2 Resolution) {
  {
    projection_render_command *Command = AddRenderCommand(Commands, projection);
    Command->AspectRatio = GetAspectRatio(Resolution);
    Command->Zoom = Zoom;
  }

  for(memsize I=0; I<UnitSelection->Count; ++I) {
    simulation_unit_id UnitID = UnitSelection->IDs[I];
    // TODO: Proper unit find based on ID (instead of index)
    // simulation_unit *Unit = GetSimulationUnit(UnitID);
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = Interpolation->Positions[UnitID].X;
    Command->Y = Interpolation->Positions[UnitID].Y;
    Command->Color = White;
    Command->HalfSize = SIMULATION_UNIT_HALF_SIZE + 10;
  }

  for(memsize I=0; I<Sim->UnitCount; ++I) {
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = Interpolation->Positions[I].X;
    Command->Y = Interpolation->Positions[I].Y;
    Command->Color = PlayerColors[Sim->Units[I].PlayerID];
    Command->HalfSize = SIMULATION_UNIT_HALF_SIZE;
  }
}

void ProcessMessageEvent(message_net_event Event, game_state *State, chunk_list *NetCmds, uusec64 Time) {
  net_message_type MessageType = UnserializeNetMessageType(Event.Message);

  switch(MessageType) {
    case net_message_type_start: {
      start_net_message StartMessage = UnserializeStartNetMessage(Event.Message);
      printf("Game got start event. PlayerCount: %zu, PlayerID: %zu\n", StartMessage.PlayerCount, StartMessage.PlayerIndex);

      InitSimulation(&State->Sim);
      for(memsize I=0; I<StartMessage.PlayerCount; ++I) {
        simulation_player_id PlayerID = SimulationCreatePlayer(&State->Sim);
        if(I == 0) {
          State->PlayerID = PlayerID;
        }
      }
      Assert(State->PlayerID != SIMULATION_UNDEFINED_PLAYER_ID);
      InitInterpolation(&State->Interpolation, &State->Sim);

      linear_allocator_context LAContext = CreateLinearAllocatorContext(&State->Allocator);
      Assert(GetLinearAllocatorFree(&State->Allocator) >= NET_MESSAGE_MAX_LENGTH);

      buffer ReplyMessage = SerializeReplyNetMessage(&State->Allocator);
      printf("Starting game and replying...\n");

      // TODO: When/if this is changed to use dynamic linear allocator
      // remember to add net command max size to free-assert above.
      memsize Length = SerializeSendNetCommand(State->CommandSerializationBuffer, ReplyMessage);
      buffer Command = {
        .Addr = State->CommandSerializationBuffer.Addr,
        .Length = Length
      };
      ChunkListWrite(NetCmds, Command);
      RestoreLinearAllocatorContext(LAContext);

      State->NextTickTime = Time + SimulationTickDuration*1000;
      State->NextExtraTickTime = Time + SimulationTickDuration*1000;
      State->Mode = game_mode_active;
      break;
    }
    case net_message_type_order_list: {
      linear_allocator_context LAContext = CreateLinearAllocatorContext(&State->Allocator);
      order_list_net_message ListMessage = UnserializeOrderListNetMessage(Event.Message, &State->Allocator);

      simulation_order_list SimOrderList;
      SimOrderList.Count = ListMessage.Count;
      SimOrderList.Orders = NULL;
      if(SimOrderList.Count != 0) {
        memsize SimOrdersSize = sizeof(simulation_order) * SimOrderList.Count;
        SimOrderList.Orders = (simulation_order*)LinearAllocate(&State->Allocator, SimOrdersSize);
        for(memsize I=0; I<SimOrderList.Count; ++I) {
          simulation_order *SimOrder = SimOrderList.Orders + I;
          net_message_order *NetOrder = ListMessage.Orders + I;
          SimOrder->PlayerID = NetOrder->PlayerID;
          SimOrder->UnitCount = NetOrder->UnitCount;
          SimOrder->Target = NetOrder->Target;

          memsize SimOrderUnitIDsSize = sizeof(simulation_unit_id) * SimOrder->UnitCount;
          SimOrder->UnitIDs = (simulation_unit_id*)LinearAllocate(&State->Allocator, SimOrderUnitIDsSize);
          for(memsize U=0; U<SimOrder->UnitCount; ++U) {
            SimOrder->UnitIDs[U] = NetOrder->UnitIDs[U];
          }
        }
      }

      buffer SimOrderListBuffer = SerializeOrderList(&SimOrderList, &State->Allocator);
      ChunkRingBufferWrite(&State->OrderListRing, SimOrderListBuffer);

      RestoreLinearAllocatorContext(LAContext);
      break;
    }
    default:
      InvalidCodePath;
  }
}

simulation_unit* FindUnit(simulation *Sim, ivec2 WorldPos) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    simulation_unit *Unit = Sim->Units + I;
    if(
      Unit->Pos.X - SIMULATION_UNIT_HALF_SIZE <= WorldPos.X &&
      Unit->Pos.X + SIMULATION_UNIT_HALF_SIZE > WorldPos.X &&
      Unit->Pos.Y - SIMULATION_UNIT_HALF_SIZE <= WorldPos.Y &&
      Unit->Pos.Y + SIMULATION_UNIT_HALF_SIZE > WorldPos.Y
    ) {
      return Unit;
    }
  }
  return NULL;
}

void ToggleUnitSelection(unit_selection *UnitSelection, simulation_unit_id ID) {
  for(memsize I=0; I<UnitSelection->Count; ++I) {
    if(UnitSelection->IDs[I] == ID) {
      UnitSelection->IDs[I] = UnitSelection->IDs[UnitSelection->Count-1];
      UnitSelection->Count--;
      return;
    }
  }
  UnitSelection->IDs[UnitSelection->Count++] = ID;
}

void ProcessMouse(simulation *Sim, linear_allocator *Allocator, simulation_player_id PlayerID, unit_selection *UnitSelection, game_mouse *Mouse, ivec2 Resolution, chunk_list *NetCmds) {
  if(Mouse->ButtonPressed && Mouse->ButtonChangeCount != 0) {
    r32 AspectRatio = GetAspectRatio(Resolution);
    ivec2 WorldPos = ConvertWindowToWorldCoors(Mouse->Pos, Resolution, AspectRatio, Zoom);
    simulation_unit *Unit = FindUnit(Sim, WorldPos);
    if(Unit != NULL && Unit->PlayerID == PlayerID) {
      ToggleUnitSelection(UnitSelection, Unit->ID);
    }
    else if(UnitSelection->Count != 0) {
      linear_allocator_context AllocatorContext = CreateLinearAllocatorContext(Allocator);
      Assert(GetLinearAllocatorFree(Allocator) >= NET_MESSAGE_MAX_LENGTH);

      buffer OrderMessage = SerializeOrderNetMessage(
        UnitSelection->IDs,
        UnitSelection->Count,
        WorldPos,
        Allocator
      );

      // TODO: Remember to add NETWORK_COMMAND_MAX_LENGTH to Free-assert() above
      // when/if this is convered to use the new serialize format.
      buffer CommandSerializationBuffer = {
        .Addr = LinearAllocate(Allocator, NETWORK_COMMAND_MAX_LENGTH),
        .Length = NETWORK_COMMAND_MAX_LENGTH
      };
      memsize Length = SerializeSendNetCommand(CommandSerializationBuffer, OrderMessage);
      buffer Command = {
        .Addr = CommandSerializationBuffer.Addr,
        .Length = Length
      };
      ChunkListWrite(NetCmds, Command);
      RestoreLinearAllocatorContext(AllocatorContext);
    }
  }
}

void RunSimulationTick(simulation *Sim, chunk_ring_buffer *OrderListRing, linear_allocator *Allocator) {
  Assert(GetChunkRingBufferUnreadCount(OrderListRing) != 0);
  buffer OrderListBuffer = ChunkRingBufferRefRead(OrderListRing);
  simulation_order_list OrderList = UnserializeOrderList(OrderListBuffer, Allocator);
  TickSimulation(Sim, &OrderList);
}

void UpdateGame(game_platform *Platform, chunk_list *NetEvents, chunk_list *NetCmds, chunk_list *RenderCmds, bool *Running, buffer Memory) {
  game_state *State = (game_state*)Memory.Addr;

  if(State->Mode == game_mode_active) {
    ProcessMouse(&State->Sim, &State->Allocator, State->PlayerID, &State->UnitSelection, Platform->Mouse, Platform->Resolution, NetCmds);
  }

  for(;;) {
    buffer Event = ChunkListRead(NetEvents);
    if(Event.Length == 0) {
      break;
    }
    net_event_type Type = UnserializeNetEventType(Event);
    switch(Type) {
      case net_event_type_connection_established:
        printf("Game got connection established!\n");
        break;
      case net_event_type_connection_lost:
        printf("Game got connection lost!\n");
        *Running = false;
        break;
      case net_event_type_connection_failed:
        printf("Game got connection failed!\n");
        *Running = false;
        break;
      case net_event_type_message: {
        message_net_event MessageEvent = UnserializeMessageNetEvent(Event);
        ProcessMessageEvent(MessageEvent, State, NetCmds, Platform->Time);
        break;
      }
      default:
        InvalidCodePath;
    }
  }

  if(State->Mode == game_mode_active) {
    if(Platform->Time >= State->NextTickTime) {
      memsize OrderListCount = GetChunkRingBufferUnreadCount(&State->OrderListRing);
      if(OrderListCount != 0) {
        RunSimulationTick(&State->Sim, &State->OrderListRing, &State->Allocator);
        OrderListCount--;
        IntSeqPush(&State->OrderListCountSeq, OrderListCount);

        if(Platform->Time >= State->NextExtraTickTime && OrderListCount != 0) {
          double CountStdDev = CalcIntSeqStdDev(&State->OrderListCountSeq);
          static const umsec32 BaseFrameLag = 200;
          memsize TargetFrameLag = BaseFrameLag/SimulationTickDuration + round(CountStdDev * 4);
          if(OrderListCount > TargetFrameLag) {
            RunSimulationTick(&State->Sim, &State->OrderListRing, &State->Allocator);
            State->NextExtraTickTime += 1000*1000;
          }
        }

        // TODO: Notify interpolation about new tick

        State->NextTickTime += SimulationTickDuration*1000;
      }
    }
    UpdateInterpolation(&State->Interpolation, &State->Sim);
    Render(&State->Sim, &State->Interpolation, &State->UnitSelection, RenderCmds, Platform->Resolution);
  }

  if(Platform->TerminationRequested) {
    printf("Requesting net shutdown...\n");

    memsize Length = SerializeShutdownNetCommand(State->CommandSerializationBuffer);
    buffer Command = {
      .Addr = State->CommandSerializationBuffer.Addr,
      .Length = Length
    };
    ChunkListWrite(NetCmds, Command);

    *Running = false;
  }
}
