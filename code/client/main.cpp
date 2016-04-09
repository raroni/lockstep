#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include "lib/assert.h"
#include "common/shared.h"
#include "common/memory.h"
#include "network_events.h"
#include "network.h"

static bool DisconnectRequested;

struct main_state {
  void *Memory;
  linear_allocator Allocator;
  pthread_t NetworkThread;
  bool Running;
};

static void HandleSigint(int signum) {
  DisconnectRequested = true;
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
  main_state MainState;
  MainState.Running = true;
  InitMemory(&MainState);

  InitNetwork();
  {
    int Result = pthread_create(&MainState.NetworkThread, 0, RunNetwork, 0);
    Assert(Result == 0);
  }

  signal(SIGINT, HandleSigint);
  while(MainState.Running) {
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
          MainState.Running = false;
          break;
        case network_event_type_connection_failed:
          printf("Game got connection failed!\n");
          MainState.Running = false;
          break;
        default:
          InvalidCodePath;
      }
    }

    if(DisconnectRequested) {
      printf("Requesting network shutdown...\n");
      ShutdownNetwork();
      MainState.Running = false;
    }
  }

  {
    printf("Waiting for thread join...\n");
    int Result = pthread_join(MainState.NetworkThread, 0);
    Assert(Result == 0);
  }

  TerminateNetwork();
  TerminateMemory(&MainState);
  printf("Gracefully terminated.\n");
  return 0;
}
