#include <stdio.h>
#include "lib/def.h"
#include "lib/assert.h"
#include "common/memory.h"
#include "common/network_messages.h"
#include "network_events.h"
#include "network_commands.h"
#include "client.h"

struct client_state {
  linear_allocator Allocator;
  buffer CommandSerializationBuffer;
};

void InitClient(client_memory *Memory) {
  Memory->Running = true;
  Memory->DisconnectRequested = false;
  client_state *State = (client_state*)Memory->MemoryPool.Addr;
  {
    void *Base = (ui8*)Memory->MemoryPool.Addr + sizeof(client_state);
    memsize Length = Memory->MemoryPool.Length - sizeof(client_state);
    InitLinearAllocator(&State->Allocator, Base, Length);
  }

  {
    buffer *B = &State->CommandSerializationBuffer;
    B->Addr = LinearAllocate(&State->Allocator, NETWORK_COMMAND_MAX_LENGTH);
    B->Length = NETWORK_COMMAND_MAX_LENGTH;
  }

}

void UpdateClient(chunk_list *NetEvents, chunk_list *NetCmds, client_memory *Memory) {
  client_state *State = (client_state*)Memory->MemoryPool.Addr;

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
        Memory->Running = false;
        break;
      case network_event_type_connection_failed:
        printf("Game got connection failed!\n");
        Memory->Running = false;
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

  if(Memory->DisconnectRequested) {
    printf("Requesting network shutdown...\n");

    memsize Length = SerializeShutdownNetworkCommand(State->CommandSerializationBuffer);
    buffer Command = {
      .Addr = State->CommandSerializationBuffer.Addr,
      .Length = Length
    };
    ChunkListWrite(NetCmds, Command);

    Memory->Running = false;
  }
}
