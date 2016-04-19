#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include "lib/assert.h"
#include "common/network_messages.h"
#include "common/memory.h"
#include "network_events.h"
#include "client.h"
#include "posix_network.h"

static bool DisconnectRequested;

struct osx_state {
  void *Memory;
  linear_allocator Allocator;
  client_state ClientState;
  pthread_t NetworkThread;
  posix_network_context NetworkContext;
};

static void HandleSigint(int signum) {
  DisconnectRequested = true;
}

void InitMemory(osx_state *State) {
  memsize MemorySize = 1024*200;
  State->Memory = malloc(MemorySize);
  InitLinearAllocator(&State->Allocator, State->Memory, MemorySize);
}

void TerminateMemory(osx_state *State) {
  TerminateLinearAllocator(&State->Allocator);
  free(State->Memory);
  State->Memory = NULL;
}

int main() {
  osx_state State;

  InitClient(&State.ClientState);
  State.ClientState.TEMP_NETWORK_CONTEXT = &State.NetworkContext;
  InitMemory(&State);

  InitNetwork(&State.NetworkContext);
  {
    int Result = pthread_create(&State.NetworkThread, 0, RunNetwork, &State.NetworkContext);
    Assert(Result == 0);
  }

  signal(SIGINT, HandleSigint);
  while(State.ClientState.Running) {
    // Gather input
    State.ClientState.DisconnectRequested = DisconnectRequested;
    UpdateClient(&State.ClientState);
    // Render();
  }

  {
    printf("Waiting for thread join...\n");
    int Result = pthread_join(State.NetworkThread, 0);
    Assert(Result == 0);
  }

  TerminateNetwork(&State.NetworkContext);
  TerminateMemory(&State);
  printf("Gracefully terminated.\n");
  return 0;
}
