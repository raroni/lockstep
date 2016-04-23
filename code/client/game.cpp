#include <stdio.h>
#include "lib/def.h"
#include "lib/assert.h"
#include "common/memory.h"
#include "common/net_messages.h"
#include "common/simulation.h"
#include "interpolation.h"
#include "net_events.h"
#include "net_commands.h"
#include "render_commands.h"
#include "game.h"

static const ui32 Red = 0x00FF0000;
static const ui32 Blue = 0x000000FF;

static const ui32 PlayerColors[] = {
  Red, Blue
};

struct game_state {
  linear_allocator Allocator;
  buffer CommandSerializationBuffer;
  memsize PlayerID;
  simulation Sim;
  interpolation Interpolation;
};

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

void Render(simulation *Sim, interpolation *Interpolation, chunk_list *Commands) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = Interpolation->Positions[I].X;
    Command->Y = Interpolation->Positions[I].Y;
    Command->Color = PlayerColors[Sim->Units[I].PlayerID];
  }
}

void ProcessMessageEvent(buffer Event, game_state *State, chunk_list *NetCmds) {
  message_net_event MessageEvent = UnserializeMessageNetEvent(Event);
  net_message_type MessageType = UnserializeNetMessageType(MessageEvent.Message);

  switch(MessageType) {
    case net_message_type_start: {
      start_net_message StartMessage = UnserializeStartNetMessage(MessageEvent.Message);
      printf("Game got start event. PlayerCount: %zu, PlayerID: %zu\n", StartMessage.PlayerCount, StartMessage.PlayerID);

      InitSimulation(&State->Sim, StartMessage.PlayerCount);
      InitInterpolation(&State->Interpolation, &State->Sim);
      State->PlayerID = StartMessage.PlayerID;

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

      Length = SerializeSendNetCommand(State->CommandSerializationBuffer, ReplyMessage);
      buffer Command = {
        .Addr = State->CommandSerializationBuffer.Addr,
        .Length = Length
      };
      ChunkListWrite(NetCmds, Command);
      break;
    }
    default:
      InvalidCodePath;
  }
}

void UpdateGame(bool TerminationRequested, chunk_list *NetEvents, chunk_list *NetCmds, chunk_list *RenderCmds, bool *Running, buffer Memory) {
  game_state *State = (game_state*)Memory.Addr;

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
        ProcessMessageEvent(Event, State, NetCmds);
        break;
      }
      default:
        InvalidCodePath;
    }
  }

  // Check if simulation update
  // Interpolation

  Render(&State->Sim, &State->Interpolation, RenderCmds);

  if(TerminationRequested) {
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
