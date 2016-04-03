#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include "../lib/assert.h"
#include "../lib/min_max.h"
#include "../lib/chunk_ring_buffer.h"
#include "../shared.h"

enum main_state {
  main_state_running,
  main_state_disconnecting,
  main_state_stopped
};

enum command_type {
  command_type_disconnect
};

struct base_command {
  command_type Type;
};

struct client {
  int FD;
};

#define CLIENT_MAX 64
struct client_set {
  client Clients[CLIENT_MAX];
  ui32 Count;
};

#define TEST_BUFFER_SIZE 4096
static ui8 TestBuffer[TEST_BUFFER_SIZE];
static int WakeReadFD;
static int WakeWriteFD;
static int HostFD;
static int ReadFDMax;
static client_set ClientSet;
static void *CommandData;
static main_state MainState;
static chunk_ring_buffer CommandList;

static void RequestWake() {
  ui8 X = 1;
  write(WakeWriteFD, &X, 1);
}

static void CheckNewReadFD(int NewFD) {
  ReadFDMax = MaxInt(ReadFDMax, NewFD);
}

static void RecalcReadFDMax() {
  ReadFDMax = 0;
  for(ui32 I=0; I<ClientSet.Count; ++I) {
    CheckNewReadFD(ClientSet.Clients[I].FD);
  }
  CheckNewReadFD(WakeReadFD);
  CheckNewReadFD(HostFD);
}

static void AddClient(client_set *Set, int FD) {
  Set->Clients[Set->Count++].FD = FD;
  CheckNewReadFD(FD);
}

static void RemoveClient(client_set *Set, ui32 Index) {
  Set->Clients[Index] = Set->Clients[Set->Count-1];
  Set->Count--;
  RecalcReadFDMax();
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

  ClientSet.Count = 0;

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

  TerminateChunkRingBuffer(&CommandList);
  free(CommandData);
  CommandData = NULL;
}

void DisconnectNetwork() {
  base_command Command;
  Command.Type = command_type_disconnect;
  ChunkRingBufferWrite(&CommandList, &Command, sizeof(Command));
  RequestWake();
}

static void ProcessCommands(main_state *MainState) {
  base_command *Command;
  memsize Length;
  while((Length = ChunkRingBufferRead(&CommandList, (void**)&Command))) {
    switch(Command->Type) {
      case command_type_disconnect: {
        for(ui32 I=0; I<ClientSet.Count; ++I) {
          int Result = shutdown(ClientSet.Clients[I].FD, SHUT_RDWR);
          Assert(Result == 0);
        }
        *MainState = main_state_disconnecting;
      }
      break;
      default:
        InvalidCodePath;
    }
  }
}

void* RunNetwork(void *Data) {
  MainState = main_state_running;

  while(MainState != main_state_stopped) {
    fd_set FDSet;
    FD_ZERO(&FDSet);
    for(ui32 I=0; I<ClientSet.Count; ++I) {
      FD_SET(ClientSet.Clients[I].FD, &FDSet);
    }
    FD_SET(HostFD, &FDSet);
    FD_SET(WakeReadFD, &FDSet);

    int SelectResult = select(ReadFDMax+1, &FDSet, NULL, NULL, NULL);
    Assert(SelectResult != -1);

    for(ui32 I=0; I<ClientSet.Count; ++I) {
      client *Client = ClientSet.Clients + I;
      if(FD_ISSET(Client->FD, &FDSet)) {
        ssize_t Result = recv(Client->FD, TestBuffer, TEST_BUFFER_SIZE, 0); // TODO: Loop until you have all
        if(Result == 0) {
          int Result = close(ClientSet.Clients[I].FD);
          Assert(Result == 0);
          RemoveClient(&ClientSet, I);
          printf("A client disconnected.\n");
          I--;
        }
        else {
          printf("Got something\n");
        }
      }
    }

    if(FD_ISSET(WakeReadFD, &FDSet)) {
      ui8 X;
      int Result = read(WakeReadFD, &X, 1);
      Assert(Result != -1);
      ProcessCommands(&MainState);
    }

    if(FD_ISSET(HostFD, &FDSet) && ClientSet.Count != CLIENT_MAX) {
      int ClientFD = accept(HostFD, NULL, NULL);
      Assert(ClientFD != -1);
      AddClient(&ClientSet, ClientFD);
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
