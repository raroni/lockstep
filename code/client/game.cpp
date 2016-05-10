#include <stdio.h>
#include <math.h>
#include "lib/def.h"
#include "lib/assert.h"
#include "lib/int_seq.h"
#include "lib/chunk_ring_buffer.h"
#include "lib/memory_arena.h"
#include "lib/min_max.h"
#include "common/net_messages.h"
#include "common/simulation.h"
#include "common/order_serialization.h"
#include "coors.h"
#include "interpolation.h"
#include "net_events.h"
#include "net_commands.h"
#include "render_commands.h"
#include "game.h"

static const ui32 DarkBlueColor = 0x002D3E50;
static const ui32 RedColor = 0x00E64E42;
static const ui32 BlueColor = 0x002F81B8;
static const ui32 PurpleColor = 0x008D48AB;
static const ui32 OrangeColor = 0x00E57F31;
static const ui32 WhiteColor = 0x00FFFFFF;
static const ui32 GreenColor = 0x0030AD63;
static const r32 Zoom = 1.0 / 1000.0;

static const ui32 PlayerColors[] = {
  RedColor, OrangeColor, BlueColor, PurpleColor
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
  memory_arena Arena;
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
    InitMemoryArena(&State->Arena, Base, Length);
  }

  {
    buffer *B = &State->CommandSerializationBuffer;
    B->Addr = MemoryArenaAllocate(&State->Arena, NET_COMMAND_MAX_LENGTH);
    B->Length = NET_COMMAND_MAX_LENGTH;
  }

  {
    usec32 WatchDuration = 10;
    memsize SamplesPerSecond = 1000 / SimulationTickDuration;
    memsize SequenceLength = SamplesPerSecond * WatchDuration;
    State->OrderListCounts = (memsize*)MemoryArenaAllocate(&State->Arena, sizeof(memsize)*SequenceLength);
    InitIntSeq(&State->OrderListCountSeq, State->OrderListCounts, SequenceLength);
  }

  {
    memsize Count = 100;
    memsize StorageSize = Count * 1024;
    void *Storage = MemoryArenaAllocate(&State->Arena, StorageSize);
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
    clear_color_render_command *Command = AddRenderCommand(Commands, clear_color);
    Command->Color = DarkBlueColor;
  }

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
    Command->X = Interpolation->Pos[UnitID].X;
    Command->Y = Interpolation->Pos[UnitID].Y;
    Command->Color = WhiteColor;
    Command->HalfSize = SIMULATION_UNIT_HALF_SIZE + 10;
  }

  for(memsize I=0; I<Sim->UnitCount; ++I) {
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = Interpolation->Pos[I].X;
    Command->Y = Interpolation->Pos[I].Y;
    Command->Color = PlayerColors[Sim->Units[I].PlayerID];
    Command->HalfSize = SIMULATION_UNIT_HALF_SIZE;
  }

  simulation_body_list *TreeList = &Sim->StaticBodyList;
  for(memsize I=0; I<TreeList->Count; ++I) {
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = TreeList->Poss[I].X;
    Command->Y = TreeList->Poss[I].Y;
    Command->Color = GreenColor;
    Command->HalfSize = SIMULATION_TREE_HALF_SIZE;
  }

  {
    projection_render_command *Command = AddRenderCommand(Commands, projection);
    Command->AspectRatio = GetAspectRatio(Resolution);
    Command->Zoom = 1.0f;
  }

  {
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = 0.0f;
    Command->Y = 0.0f;
    Command->Color = OrangeColor;
    Command->HalfSize = 0.5f;
  }
}

void ProcessMessageEvent(message_net_event Event, game_state *State, chunk_list *NetCmds, uusec64 Time) {
  net_message_type MessageType = UnserializeNetMessageType(Event.Message);

  switch(MessageType) {
    case net_message_type_start: {
      start_net_message StartMessage = UnserializeStartNetMessage(Event.Message);
      printf("Game got start event. PlayerCount: %zu, PlayerID: %zu\n", StartMessage.PlayerCount, StartMessage.PlayerIndex);

      InitSimulation(&State->Sim, &State->Arena);
      for(memsize I=0; I<StartMessage.PlayerCount; ++I) {
        simulation_player_id PlayerID = SimulationCreatePlayer(&State->Sim);
        if(I == StartMessage.PlayerIndex) {
          State->PlayerID = PlayerID;
        }
      }
      Assert(State->PlayerID != SIMULATION_UNDEFINED_PLAYER_ID);
      InitInterpolation(&State->Interpolation, &State->Sim);

      memory_arena_checkpoint ArenaCheckpoint = CreateMemoryArenaCheckpoint(&State->Arena);
      Assert(GetMemoryArenaFree(&State->Arena) >= NET_MESSAGE_MAX_LENGTH + NET_COMMAND_MAX_LENGTH);

      buffer ReplyMessage = SerializeReplyNetMessage(&State->Arena);
      printf("Starting game and replying...\n");

      buffer Command = SerializeSendNetCommand(ReplyMessage, &State->Arena);
      ChunkListWrite(NetCmds, Command);
      ReleaseMemoryArenaCheckpoint(ArenaCheckpoint);

      State->NextTickTime = Time + SimulationTickDuration*1000;
      State->NextExtraTickTime = Time + SimulationTickDuration*1000;
      State->Mode = game_mode_active;
      break;
    }
    case net_message_type_order_list: {
      memory_arena_checkpoint ArenaCheckpoint = CreateMemoryArenaCheckpoint(&State->Arena);
      order_list_net_message ListMessage = UnserializeOrderListNetMessage(Event.Message, &State->Arena);

      simulation_order_list SimOrderList;
      SimOrderList.Count = ListMessage.Count;
      SimOrderList.Orders = NULL;
      if(SimOrderList.Count != 0) {
        memsize SimOrdersSize = sizeof(simulation_order) * SimOrderList.Count;
        SimOrderList.Orders = (simulation_order*)MemoryArenaAllocate(&State->Arena, SimOrdersSize);
        for(memsize I=0; I<SimOrderList.Count; ++I) {
          simulation_order *SimOrder = SimOrderList.Orders + I;
          net_message_order *NetOrder = ListMessage.Orders + I;
          SimOrder->PlayerID = NetOrder->PlayerID;
          SimOrder->UnitCount = NetOrder->UnitCount;
          SimOrder->Target = NetOrder->Target;

          memsize SimOrderUnitIDsSize = sizeof(simulation_unit_id) * SimOrder->UnitCount;
          SimOrder->UnitIDs = (simulation_unit_id*)MemoryArenaAllocate(&State->Arena, SimOrderUnitIDsSize);
          for(memsize U=0; U<SimOrder->UnitCount; ++U) {
            SimOrder->UnitIDs[U] = NetOrder->UnitIDs[U];
          }
        }
      }

      buffer SimOrderListBuffer = SerializeOrderList(&SimOrderList, &State->Arena);
      ChunkRingBufferWrite(&State->OrderListRing, SimOrderListBuffer);

      ReleaseMemoryArenaCheckpoint(ArenaCheckpoint);
      break;
    }
    default:
      InvalidCodePath;
  }
}

simulation_unit* FindUnit(simulation *Sim, ivec2 WorldPos) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    simulation_unit *Unit = Sim->Units + I;
    ivec2 Pos = SimulationGetUnitPos(Sim, Unit);
    if(
      Pos.X - SIMULATION_UNIT_HALF_SIZE <= WorldPos.X &&
      Pos.X + SIMULATION_UNIT_HALF_SIZE > WorldPos.X &&
      Pos.Y - SIMULATION_UNIT_HALF_SIZE <= WorldPos.Y &&
      Pos.Y + SIMULATION_UNIT_HALF_SIZE > WorldPos.Y
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
  const int UnitSelectionMax = sizeof(UnitSelection->IDs) / sizeof(UnitSelection->IDs[0]);
  if(UnitSelection->Count != UnitSelectionMax) {
    UnitSelection->IDs[UnitSelection->Count++] = ID;
  }
}

void ProcessMouse(simulation *Sim, memory_arena *Arena, simulation_player_id PlayerID, unit_selection *UnitSelection, game_mouse *Mouse, ivec2 Resolution, chunk_list *NetCmds) {
  if(Mouse->ButtonPressed && Mouse->ButtonChangeCount != 0) {
    r32 AspectRatio = GetAspectRatio(Resolution);
    ivec2 WorldPos = ConvertWindowToWorldCoors(Mouse->Pos, Resolution, AspectRatio, Zoom);
    simulation_unit *Unit = FindUnit(Sim, WorldPos);
    if(Unit != NULL && Unit->PlayerID == PlayerID) {
      ToggleUnitSelection(UnitSelection, Unit->ID);
    }
    else if(UnitSelection->Count != 0) {
      memory_arena_checkpoint ArenaCheckpoint = CreateMemoryArenaCheckpoint(Arena);
      Assert(GetMemoryArenaFree(Arena) >= NET_MESSAGE_MAX_LENGTH + NET_COMMAND_MAX_LENGTH);

      buffer OrderMessage = SerializeOrderNetMessage(
        UnitSelection->IDs,
        UnitSelection->Count,
        WorldPos,
        Arena
      );
      buffer Command = SerializeSendNetCommand(OrderMessage, Arena);
      ChunkListWrite(NetCmds, Command);
      ReleaseMemoryArenaCheckpoint(ArenaCheckpoint);
    }
  }
}

void RunSimulationTick(simulation *Sim, chunk_ring_buffer *OrderListRing, memory_arena *Arena) {
  Assert(GetChunkRingBufferUnreadCount(OrderListRing) != 0);
  buffer OrderListBuffer = ChunkRingBufferRefRead(OrderListRing);
  simulation_order_list OrderList = UnserializeOrderList(OrderListBuffer, Arena);
  TickSimulation(Sim, &OrderList);
}

void UpdateGame(game_platform *Platform, chunk_list *NetEvents, chunk_list *NetCmds, chunk_list *RenderCmds, bool *Running, buffer Memory) {
  game_state *State = (game_state*)Memory.Addr;

  if(State->Mode == game_mode_active) {
    ProcessMouse(&State->Sim, &State->Arena, State->PlayerID, &State->UnitSelection, Platform->Mouse, Platform->Resolution, NetCmds);
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
        RunSimulationTick(&State->Sim, &State->OrderListRing, &State->Arena);
        OrderListCount--;
        IntSeqPush(&State->OrderListCountSeq, OrderListCount);

        if(Platform->Time >= State->NextExtraTickTime && OrderListCount != 0) {
          double CountStdDev = CalcIntSeqStdDev(&State->OrderListCountSeq);
          static const umsec32 BaseFrameLag = 200;
          memsize TargetFrameLag = BaseFrameLag/SimulationTickDuration + round(CountStdDev * 4);
          if(OrderListCount > TargetFrameLag) {
            RunSimulationTick(&State->Sim, &State->OrderListRing, &State->Arena);
            State->NextExtraTickTime += 1000*1000;
          }
        }

        ReloadInterpolation(&State->Interpolation, &State->Sim);

        State->NextTickTime += SimulationTickDuration*1000;
      }
    }

    r32 TickProgress;
    {
      uusec64 TickDuration = SimulationTickDuration*1000;
      uusec64 TimeUntilNextTick = State->NextTickTime - Platform->Time;
      uusec64 TimeSpentInCurrentTick = TickDuration - TimeUntilNextTick;
      TickProgress = (r32)TimeSpentInCurrentTick / (r32)TickDuration;
      TickProgress = MinR32(TickProgress, 1.0f);
    }
    UpdateInterpolation(&State->Interpolation, &State->Sim, TickProgress);
    Render(&State->Sim, &State->Interpolation, &State->UnitSelection, RenderCmds, Platform->Resolution);
  }

  if(Platform->TerminationRequested) {
    printf("Requesting net shutdown...\n");

    memory_arena_checkpoint ArenaCheckpoint = CreateMemoryArenaCheckpoint(&State->Arena);
    Assert(GetMemoryArenaFree(&State->Arena) >= NET_COMMAND_MAX_LENGTH);
    buffer Command = SerializeShutdownNetCommand(&State->Arena);
    ChunkListWrite(NetCmds, Command);
    ReleaseMemoryArenaCheckpoint(ArenaCheckpoint);

    *Running = false;
  }
}
