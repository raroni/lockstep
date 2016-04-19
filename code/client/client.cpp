#include <stdio.h>
#include "lib/def.h"
#include "lib/assert.h"
#include "common/network_messages.h"
#include "network_events.h"
#include "network_commands.h"
#include "client.h"

// TODO: Should not use stdlib (use linear_allocator instead).
#include <stdlib.h>

static buffer CommandSerializationBuffer;

static buffer CreateBuffer(memsize Length) {
  buffer B;
  B.Addr = malloc(Length);
  Assert(B.Addr != NULL);
  B.Length = Length;
  return B;
}

static void DestroyBuffer(buffer *B) {
  free(B->Addr);
  B->Addr = NULL;
  B->Length = 0;
}

void InitClient(client_state *State) {
  State->Running = true;
  State->DisconnectRequested = false;
  State->CommandSerializationBuffer = CreateBuffer(NETWORK_COMMAND_MAX_LENGTH);
}

void UpdateClient(chunk_list *NetCmds, client_state *State) {
  memsize Length;
  static ui8 ReadBufferBlock[NETWORK_EVENT_MAX_LENGTH];
  static buffer ReadBuffer = {
    .Addr = &ReadBufferBlock,
    .Length = sizeof(ReadBufferBlock)
  };
  while((Length = ReadNetworkEvent(State->TEMP_NETWORK_CONTEXT, ReadBuffer))) {
    network_event_type Type = UnserializeNetworkEventType(ReadBuffer);
    switch(Type) {
      case network_event_type_connection_established:
        printf("Game got connection established!\n");
        break;
      case network_event_type_connection_lost:
        printf("Game got connection lost!\n");
        State->Running = false;
        break;
      case network_event_type_connection_failed:
        printf("Game got connection failed!\n");
        State->Running = false;
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

  if(State->DisconnectRequested) {
    printf("Requesting network shutdown...\n");
    ShutdownNetwork(State->TEMP_NETWORK_CONTEXT);
    State->Running = false;
  }
}

void TerminateClient(client_state *State) {
  DestroyBuffer(&State->CommandSerializationBuffer);
}
