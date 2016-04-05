#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include "lib/assert.h"
#include "lib/min_max.h"
#include "lib/chunk_ring_buffer.h"
#include "common/shared.h"
#include "client_set.h"
#include "network_events.h"
#include "network_commands.h"
#include "network.h"

enum main_state {
  main_state_running,
  main_state_disconnecting,
  main_state_stopped
};

#define RECEIVE_BUFFER_SIZE 4096
static ui8 ReceiveBuffer[RECEIVE_BUFFER_SIZE];
static ui8 EventOutBuffer[NETWORK_EVENT_MAX_LENGTH];
static ui8 CommandSerializationBuffer[COMMAND_MAX_LENGTH];
static int WakeReadFD;
static int WakeWriteFD;
static int HostFD;
static int ReadFDMax;
static client_set ClientSet;
static void *CommandData;
static void *EventData;
static main_state MainState;
static chunk_ring_buffer CommandList;
static chunk_ring_buffer EventBuffer;

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

  memsize CommandDataLength = 1024*100;
  CommandData = malloc(CommandDataLength);
  InitChunkRingBuffer(&CommandList, 50, CommandData, CommandDataLength);

  memsize EventDataLength = 1024*100;
  EventData = malloc(EventDataLength);
  InitChunkRingBuffer(&EventBuffer, 50, EventData, EventDataLength);

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
  close(WakeReadFD);
  close(WakeWriteFD);

  int Result = close(HostFD);
  Assert(Result == 0);

  TerminateClientSet(&ClientSet);

  TerminateChunkRingBuffer(&CommandList);
  free(CommandData);
  CommandData = NULL;

  TerminateChunkRingBuffer(&EventBuffer);
  free(EventData);
  EventData = NULL;
}

void DisconnectNetwork() {
  memsize Length = SerializeDisconnectNetworkCommand(CommandSerializationBuffer, sizeof(CommandSerializationBuffer));
  ChunkRingBufferWrite(&CommandList, CommandSerializationBuffer, Length);
  RequestWake();
}

static void ProcessCommands(main_state *MainState) {
  memsize Length;
  static ui8 CommandUnserializationBuffer[COMMAND_MAX_LENGTH];
  while((Length = ChunkRingBufferRead(&CommandList, CommandUnserializationBuffer, sizeof(CommandUnserializationBuffer)))) {
    network_command_type Type = UnserializeNetworkCommandType(CommandUnserializationBuffer, sizeof(CommandUnserializationBuffer));
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
        broadcast_network_command Command = UnserializeBroadcastNetworkCommand(CommandUnserializationBuffer, Length);
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

memsize ReadNetworkEvent(void *Buffer, memsize MaxLength) {
  return ChunkRingBufferRead(&EventBuffer, Buffer, MaxLength);
}

void NetworkBroadcast(client_id *IDs, memsize IDCount, void *Message, memsize MessageLength) {
  memsize Length = SerializeBroadcastNetworkCommand(
    IDs,
    IDCount,
    Message,
    MessageLength,
    CommandSerializationBuffer,
    sizeof(CommandSerializationBuffer)
  );
  ChunkRingBufferWrite(&CommandList, CommandSerializationBuffer, Length);
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
          ssize_t Result = recv(Client->FD, ReceiveBuffer, RECEIVE_BUFFER_SIZE, 0);
          if(Result == 0) {
            int Result = close(Client->FD);
            Assert(Result != -1);
            DestroyClient(&Iterator);
            memsize Length = SerializeDisconnectNetworkEvent(Client->ID, EventOutBuffer, sizeof(EventOutBuffer));
            ChunkRingBufferWrite(&EventBuffer, EventOutBuffer, Length);
            printf("A client disconnected.\n");
          }
          else {
            ByteRingBufferWrite(&Client->InBuffer, ReceiveBuffer, Result);
            printf("Got something\n");
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
      memsize Length = SerializeConnectNetworkEvent(Client->ID, EventOutBuffer, sizeof(EventOutBuffer));
      ChunkRingBufferWrite(&EventBuffer, EventOutBuffer, Length);
      printf("Someone connected!\n");
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
