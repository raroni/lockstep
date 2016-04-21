#include <stdio.h>
#include "lib/def.h"
#include "lib/assert.h"
#include "common/memory.h"
#include "common/network_messages.h"
#include "common/simulation.h"
#include "network_events.h"
#include "network_commands.h"
#include "render_commands.h"
#include "client.h"

static const ui32 Red = 0x000000FF;
static const ui32 Blue = 0x00FF0000;

struct client_state {
  linear_allocator Allocator;
  buffer CommandSerializationBuffer;
  simulation Sim;
};

void InitClient(buffer Memory) {
  client_state *State = (client_state*)Memory.Addr;
  {
    void *Base = (ui8*)Memory.Addr + sizeof(client_state);
    memsize Length = Memory.Length - sizeof(client_state);
    InitLinearAllocator(&State->Allocator, Base, Length);
  }

  {
    buffer *B = &State->CommandSerializationBuffer;
    B->Addr = LinearAllocate(&State->Allocator, NETWORK_COMMAND_MAX_LENGTH);
    B->Length = NETWORK_COMMAND_MAX_LENGTH;
  }

  InitSimulation(&State->Sim);
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

void Render(chunk_list *Commands) {
  draw_square_render_command *Command1 = AddRenderCommand(Commands, draw_square);
  Command1->X = 100;
  Command1->Y = 100;
  Command1->Color = Red;

  draw_square_render_command *Command2 = AddRenderCommand(Commands, draw_square);
  Command2->X = -100;
  Command2->Y = -100;
  Command2->Color = Blue;
}

void UpdateClient(bool TerminationRequested, chunk_list *NetEvents, chunk_list *NetCmds, chunk_list *RenderCmds, bool *Running, buffer Memory) {
  client_state *State = (client_state*)Memory.Addr;

  for(;;) {
    buffer Event = ChunkListRead(NetEvents);
    if(Event.Length == 0) {
      break;
    }
    network_event_type Type = UnserializeNetworkEventType(Event);
    switch(Type) {
      case network_event_type_connection_established:
        printf("Game got connection established!\n");
        break;
      case network_event_type_connection_lost:
        printf("Game got connection lost!\n");
        *Running = false;
        break;
      case network_event_type_connection_failed:
        printf("Game got connection failed!\n");
        *Running = false;
        break;
      case network_event_type_start: {
        printf("Game got start event!\n");

        static ui8 TempBufferBlock[MAX_MESSAGE_LENGTH];
        buffer TempBuffer = {
          .Addr = TempBufferBlock,
          .Length = sizeof(TempBufferBlock)
        };
        memsize Length = SerializeReplyNetworkMessage(TempBuffer);
        buffer Message = {
          .Addr = TempBuffer.Addr,
          .Length = Length
        };
        printf("Starting game and replying...\n");

        Length = SerializeSendNetworkCommand(State->CommandSerializationBuffer, Message);
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

  // Check if simulation update
  // Interpolation

  Render(RenderCmds);

  if(TerminationRequested) {
    printf("Requesting network shutdown...\n");

    memsize Length = SerializeShutdownNetworkCommand(State->CommandSerializationBuffer);
    buffer Command = {
      .Addr = State->CommandSerializationBuffer.Addr,
      .Length = Length
    };
    ChunkListWrite(NetCmds, Command);

    *Running = false;
  }
}
