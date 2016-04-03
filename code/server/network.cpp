#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
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

static int WakeReadFD;
static int WakeWriteFD;
static void *CommandData;
static main_state MainState;
static chunk_ring_buffer CommandList;

static void RequestWake() {
  ui8 X = 1;
  write(WakeWriteFD, &X, 1);
}

void InitNetwork2() {
  int FDs[2];
  pipe(FDs);
  WakeReadFD = FDs[0];
  WakeWriteFD = FDs[1];

  memsize CommandDataLength = 1024*100;
  CommandData = malloc(CommandDataLength);
  InitChunkRingBuffer(&CommandList, 50, CommandData, CommandDataLength);
}

void TerminateNetwork2() {
  TerminateChunkRingBuffer(&CommandList);
  free(CommandData);
  CommandData = NULL;
}

void DisconnectNetwork2() {
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
      case command_type_disconnect:
        *MainState = main_state_disconnecting;
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
    // for(ui32 I=0; I<Network.ClientSet.Count; ++I) {
    //   FD_SET(Network.ClientSet.Clients[I].FD, &ClientFDSet);
    // }
    FD_SET(WakeReadFD, &FDSet);

    // todo: Max proper +1 here
    int SelectResult = select(WakeReadFD+1, &FDSet, NULL, NULL, NULL);
    Assert(SelectResult != -1);

    if(FD_ISSET(WakeReadFD, &FDSet)) {
      ui8 X;
      int Result = read(WakeReadFD, &X, 1);
      Assert(Result != -1);
      ProcessCommands(&MainState);
    }

    if(MainState == main_state_disconnecting) {
      // This is fake. Make proper.
      printf("No more clients. Stopping.\n");
      MainState = main_state_stopped;
    }
  }

  close(WakeReadFD);
  close(WakeWriteFD);

  return NULL;
}
