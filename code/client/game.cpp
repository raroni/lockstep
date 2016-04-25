#include <stdio.h>
#include <math.h>
#include "lib/def.h"
#include "lib/assert.h"
#include "lib/int_seq.h"
#include "lib/chunk_ring_buffer.h"
#include "common/memory.h"
#include "common/net_messages.h"
#include "common/simulation.h"
#include "coors.h"
#include "interpolation.h"
#include "net_events.h"
#include "net_commands.h"
#include "render_commands.h"
#include "game.h"

static const ui32 Red = 0x00FF0000;
static const ui32 Blue = 0x000000FF;
static const r32 Zoom = 1.0 / 1000.0;

static const ui32 PlayerColors[] = {
  Red, Blue
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

void Render(simulation *Sim, interpolation *Interpolation, chunk_list *Commands, ivec2 Resolution) {
  {
    projection_render_command *Command = AddRenderCommand(Commands, projection);
    Command->AspectRatio = GetAspectRatio(Resolution);
    Command->Zoom = Zoom;
  }

  for(memsize I=0; I<Sim->UnitCount; ++I) {
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = Interpolation->Positions[I].X;
    Command->Y = Interpolation->Positions[I].Y;
    Command->Color = PlayerColors[Sim->Units[I].PlayerID];
  }
}

void ProcessMessageEvent(buffer Event, game_state *State, chunk_list *NetCmds, uusec64 Time) {
  message_net_event MessageEvent = UnserializeMessageNetEvent(Event);
  net_message_type MessageType = UnserializeNetMessageType(MessageEvent.Message);

  switch(MessageType) {
    case net_message_type_start: {
      start_net_message StartMessage = UnserializeStartNetMessage(MessageEvent.Message);
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

      static ui8 TempBufferBlock[MAX_MESSAGE_LENGTH];
      buffer TempBuffer = {
        .Addr = TempBufferBlock,
        .Length = sizeof(TempBufferBlock)
      };
      memsize Length = SerializeReplyNetMessage(TempBuffer);
      buffer ReplyMessage = {
        .Addr = TempBuffer.Addr,
        .Length = Length
      };
      printf("Starting game and replying...\n");

      State->NextTickTime = Time + SimulationTickDuration*1000;
      State->NextExtraTickTime = Time + SimulationTickDuration*1000;

      Length = SerializeSendNetCommand(State->CommandSerializationBuffer, ReplyMessage);
      buffer Command = {
        .Addr = State->CommandSerializationBuffer.Addr,
        .Length = Length
      };
      ChunkListWrite(NetCmds, Command);
      break;
    }
    case net_message_type_order_list:
      // TODO: Handle
      break;
    default:
      InvalidCodePath;
  }
}

void ProcessMouse(game_mouse *Mouse, ivec2 Resolution) {
  if(Mouse->ButtonPressed && Mouse->ButtonChangeCount != 0) {
    r32 AspectRatio = GetAspectRatio(Resolution);
    ivec2 WorldPos = ConvertWindowToWorldCoors(Mouse->Pos, Resolution, AspectRatio, Zoom);
    // TODO: Collide with units
  }
}

void UpdateGame(game_platform *Platform, chunk_list *NetEvents, chunk_list *NetCmds, chunk_list *RenderCmds, bool *Running, buffer Memory) {
  game_state *State = (game_state*)Memory.Addr;

  ProcessMouse(Platform->Mouse, Platform->Resolution);

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
        ProcessMessageEvent(Event, State, NetCmds, Platform->Time);
        break;
      }
      default:
        InvalidCodePath;
    }
  }

  if(Platform->Time >= State->NextTickTime) {
    memsize OrderListCount = GetChunkRingBufferUnreadCount(&State->OrderListRing);
    if(OrderListCount != 0) {
      // TODO: extract order set and pass to tick
      OrderListCount--;

      simulation_order_list DummyOrderList;
      DummyOrderList.Count = 0;
      TickSimulation(&State->Sim, &DummyOrderList);
      IntSeqPush(&State->OrderListCountSeq, OrderListCount);

      if(Platform->Time >= State->NextExtraTickTime) {
        double CountStdDev = CalcIntSeqStdDev(&State->OrderListCountSeq);
        static const umsec32 BaseFrameLag = 200;
        memsize TargetFrameLag = BaseFrameLag/SimulationTickDuration + round(CountStdDev * 4);
        if(OrderListCount > TargetFrameLag) {
          // TODO: extract order set and pass to tick
          TickSimulation(&State->Sim, &DummyOrderList);
          State->NextExtraTickTime += 1000*1000;
        }
      }

      // check for extra tick-sim here

      // TODO: Notify interpolation about new tick

      State->NextTickTime += SimulationTickDuration*1000;
    }
  }
  // TODO: Perform interpolation

  Render(&State->Sim, &State->Interpolation, RenderCmds, Platform->Resolution);

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
