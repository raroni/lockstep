#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "../shared.h"
#include "../memory.h"
#include "network_client.h"

static bool Running;

struct main_state {
  void *Memory;
  linear_allocator Allocator;
};

static void HandleSigint(int signum) {
  Running = false;
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
  signal(SIGINT, HandleSigint);

  main_state MainState;
  InitMemory(&MainState);
  InitNetwork();

  Running = true;
  while(Running) {
    if(Network.State != network_state_inactive) {
      UpdateNetwork();
    }
    else {
      break;
    }
  }

  TerminateNetwork();
  TerminateMemory(&MainState);
  printf("Gracefully terminated.\n");
  return 0;
}
