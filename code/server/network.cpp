#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include "lib/assert.h"
#include "lib/min_max.h"
#include "lib/chunk_ring_buffer.h"
#include "common/network.h"
#include "client_set.h"
#include "network_events.h"
#include "network_commands.h"
#include "network.h"

enum main_state {
  main_state_running,
  main_state_disconnecting,
  main_state_stopped
};

static ui8 ReceiveBufferBlock[1024*10];
static buffer ReceiveBuffer = {
  .Addr = &ReceiveBufferBlock,
  .Length = sizeof(ReceiveBufferBlock)
};

static ui8 EventOutBufferBlock[NETWORK_EVENT_MAX_LENGTH];
static buffer EventOutBuffer = {
  .Addr = &EventOutBufferBlock,
  .Length = sizeof(EventOutBufferBlock)
};
static ui8 CommandSerializationBufferBlock[COMMAND_MAX_LENGTH];
static buffer CommandSerializationBuffer = {
  .Addr = &CommandSerializationBufferBlock,
  .Length = sizeof(CommandSerializationBufferBlock)
};
static int WakeReadFD;
static int WakeWriteFD;
static int HostFD;
static int ReadFDMax;
static client_set ClientSet;
static void *CommandBufferAddr;
static void *EventBufferAddr;
static main_state MainState;
static chunk_ring_buffer CommandRing;
static chunk_ring_buffer EventRing;

static void RequestWake() {
  ui8 X = 1;
  write(WakeWriteFD, &X, 1);
}

static void CheckNewReadFD(int NewFD) {
  ReadFDMax = MaxInt(ReadFDMax, NewFD);
}

static void RecalcReadFDMax() {
  ReadFDMax = 0;
  client_set_iterator Iterator = CreateClientSetIterator(&ClientSet);
  while(AdvanceClientSetIterator(&Iterator)) {
    CheckNewReadFD(Iterator.Client->FD);
  }
  CheckNewReadFD(WakeReadFD);
  CheckNewReadFD(HostFD);
}

void InitNetwork() {
  ReadFDMax = 0;

  int FDs[2];
  pipe(FDs);
  WakeReadFD = FDs[0];
  WakeWriteFD = FDs[1];
  CheckNewReadFD(WakeReadFD);

  {
    memsize CommandBufferLength = 1024*100;
    CommandBufferAddr = malloc(CommandBufferLength);
    buffer CommandBuffer = {
      .Addr = CommandBufferAddr,
      .Length = CommandBufferLength
    };
    InitChunkRingBuffer(&CommandRing, 50, CommandBuffer);
  }

  {
    memsize EventBufferLength = 1024*100;
    EventBufferAddr = malloc(EventBufferLength);
    buffer EventBuffer = {
      .Addr = EventBufferAddr,
      .Length = EventBufferLength
    };
    InitChunkRingBuffer(&EventRing, 50, EventBuffer);
  }

  InitClientSet(&ClientSet);

  HostFD = socket(PF_INET, SOCK_STREAM, 0);
  Assert(HostFD != -1);
  CheckNewReadFD(HostFD);
  fcntl(HostFD, F_SETFL, O_NONBLOCK);

  struct sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_len = sizeof(Address);
  Address.sin_family = AF_INET;
  Address.sin_port = htons(4321);
  Address.sin_addr.s_addr = INADDR_ANY;

  int BindResult = bind(HostFD, (struct sockaddr *)&Address, sizeof(Address));
  Assert(BindResult != -1);

  int ListenResult = listen(HostFD, 5);
  Assert(ListenResult == 0);
}

void TerminateNetwork() {
  int Result = close(WakeReadFD);
  Assert(Result == 0);
  Result = close(WakeWriteFD);
  Assert(Result == 0);

  Result = close(HostFD);
  Assert(Result == 0);

  TerminateClientSet(&ClientSet);

  TerminateChunkRingBuffer(&CommandRing);
  free(CommandBufferAddr);
  CommandBufferAddr = NULL;

  TerminateChunkRingBuffer(&EventRing);
  free(EventBufferAddr);
  EventBufferAddr = NULL;
}

void DisconnectNetwork() {
  memsize Length = SerializeDisconnectNetworkCommand(CommandSerializationBuffer);
  buffer Command = {
    .Addr = CommandSerializationBuffer.Addr,
    .Length = Length
  };
  ChunkRingBufferWrite(&CommandRing, Command);
  RequestWake();
}

static void ProcessCommands(main_state *MainState) {
  memsize Length;
  static ui8 BufferStorage[COMMAND_MAX_LENGTH];
  static buffer Buffer = { .Addr = BufferStorage, .Length = COMMAND_MAX_LENGTH };
  while((Length = ChunkRingBufferRead(&CommandRing, Buffer))) {
    network_command_type Type = UnserializeNetworkCommandType(Buffer);
    switch(Type) {
      case network_command_type_disconnect: {
        client_set_iterator Iterator = CreateClientSetIterator(&ClientSet);
        while(AdvanceClientSetIterator(&Iterator)) {
          int Result = shutdown(Iterator.Client->FD, SHUT_RDWR);
          Assert(Result == 0);
        }
        *MainState = main_state_disconnecting;
        break;
      }
      case network_command_type_broadcast: {
        broadcast_network_command Command = UnserializeBroadcastNetworkCommand(Buffer);
        for(memsize I=0; I<Command.ClientIDCount; ++I) {
          client *Client = FindClientByID(&ClientSet, Command.ClientIDs[I]);
          if(Client) {
            printf("Broadcasted to client id %zu\n", Command.ClientIDs[I]);
            ssize_t Result = send(Client->FD, Command.Message, Command.MessageLength, 0);
            Assert(Result != -1);
          }
        }

        break;
      }
      break;
      default:
        InvalidCodePath;
    }
  }
}

memsize ReadNetworkEvent(buffer Buffer) {
  return ChunkRingBufferRead(&EventRing, Buffer);
}

void NetworkBroadcast(client_id *IDs, memsize IDCount, buffer Message) {
  memsize Length = SerializeBroadcastNetworkCommand(
    IDs,
    IDCount,
    Message,
    CommandSerializationBuffer
  );
  buffer Command = {
    .Addr = CommandSerializationBuffer.Addr,
    .Length = Length
  };
  ChunkRingBufferWrite(&CommandRing, Command);
  RequestWake();
}

void* RunNetwork(void *Data) {
  MainState = main_state_running;

  while(MainState != main_state_stopped) {
    fd_set FDSet;
    FD_ZERO(&FDSet);
    {
      client_set_iterator Iterator = CreateClientSetIterator(&ClientSet);
      while(AdvanceClientSetIterator(&Iterator)) {
        FD_SET(Iterator.Client->FD, &FDSet);
      }
    }
    FD_SET(HostFD, &FDSet);
    FD_SET(WakeReadFD, &FDSet);

    int SelectResult = select(ReadFDMax+1, &FDSet, NULL, NULL, NULL);
    Assert(SelectResult != -1);

    {
      client_set_iterator Iterator = CreateClientSetIterator(&ClientSet);
      while(AdvanceClientSetIterator(&Iterator)) {
        client *Client = Iterator.Client;
        if(FD_ISSET(Client->FD, &FDSet)) {
          ssize_t Result = NetworkReceive(Client->FD, ReceiveBuffer);
          if(Result == 0) {
            int Result = close(Client->FD);
            Assert(Result != -1);
            DestroyClient(&Iterator);
            memsize Length = SerializeDisconnectNetworkEvent(Client->ID, EventOutBuffer);
            buffer Event = {
              .Addr = EventOutBuffer.Addr,
              .Length = Length
            };
            ChunkRingBufferWrite(&EventRing, Event);
            printf("A client disconnected.\n");
          }
          else {
            buffer Input;
            Input.Addr = ReceiveBuffer.Addr;
            Input.Length = Result;
            printf("Write to %p\n", &Client->InBuffer);
            ByteRingBufferWrite(&Client->InBuffer, Input);
          }
        }
        RecalcReadFDMax();
      }
    }

    if(FD_ISSET(WakeReadFD, &FDSet)) {
      ui8 X;
      int Result = read(WakeReadFD, &X, 1);
      Assert(Result != -1);
      ProcessCommands(&MainState);
    }

    if(
      FD_ISSET(HostFD, &FDSet) &&
      ClientSet.Count != CLIENT_SET_MAX &&
      MainState == main_state_running
    ) {
      int ClientFD = accept(HostFD, NULL, NULL);
      Assert(ClientFD != -1);
      client *Client = CreateClient(&ClientSet, ClientFD);
      CheckNewReadFD(ClientFD);
      memsize Length = SerializeConnectNetworkEvent(Client->ID, EventOutBuffer);
      buffer Event = {
        .Addr = EventOutBuffer.Addr,
        .Length = Length
      };
      ChunkRingBufferWrite(&EventRing, Event);
    }

    if(MainState == main_state_disconnecting) {
      if(ClientSet.Count == 0) {
        printf("No more clients. Stopping.\n");
        MainState = main_state_stopped;
      }
    }
  }

  return NULL;
}
