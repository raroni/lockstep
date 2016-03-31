#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include "../shared.h"
#include "../memory.h"
#include "shared.h"
#include "network.h"
#include "network_client.h"

struct main_state {
  void *Memory;
  linear_allocator Allocator;
  pthread_t NetworkThread;
};

static void HandleSigint(int signum) {
  ClientRunning = false;
}

void InitMemory(main_state *State) {
  memsize MemorySize = 1024*200;
  State->Memory = malloc(MemorySize);
  InitLinearAllocator(&State->Allocator, State->Memory, MemorySize);
}

void TerminateMemory(main_state *State) {
  TerminateLinearAllocator(&State->Allocator);
  free(State->Memory);
  State->Memory = NULL;
}

int main() {
  ClientRunning = true;
  signal(SIGINT, HandleSigint);

  main_state MainState;
  InitMemory(&MainState);
  InitNetwork();
  pthread_create(&MainState.NetworkThread, 0, RunNetwork, 0);

  while(ClientRunning) {
    if(Network.State != network_state_inactive) {
      UpdateNetwork();
    }
    else {
      ClientRunning = false;
    }
  }

  pthread_join(MainState.NetworkThread, 0);

  TerminateNetwork();
  TerminateMemory(&MainState);
  printf("Gracefully terminated.\n");
  return 0;
}
