#include <stdio.h>
#include "lib/def.h"
#include "lib/assert.h"
#include "common/network_messages.h"
#include "posix_network.h"
#include "network_events.h"
#include "client.h"

void InitClient(client_state *State) {
  State->Running = true;
  State->DisconnectRequested = false;
}

void UpdateClient(client_state *State) {
  memsize Length;
  static ui8 ReadBufferBlock[NETWORK_EVENT_MAX_LENGTH];
  static buffer ReadBuffer = {
    .Addr = &ReadBufferBlock,
    .Length = sizeof(ReadBufferBlock)
  };
  while((Length = ReadNetworkEvent(ReadBuffer))) {
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
        NetworkSend(Message);
        break;
      }
      default:
        InvalidCodePath;
    }
  }

  if(State->DisconnectRequested) {
    printf("Requesting network shutdown...\n");
    ShutdownNetwork();
    State->Running = false;
  }
}
