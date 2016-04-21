#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include "lib/assert.h"
#include "lib/chunk_list.h"
#include "common/memory.h"
#include "common/network_messages.h"
#include "network_commands.h"
#include "network_events.h"
#include "server.h"
#include "network.h"

static bool TerminationRequested;

struct osx_state {
  pthread_t NetworkThread;
  chunk_list NetCommandList;
  chunk_list NetEventList;
  buffer ServerMemory;
  bool Running;
  void *Memory;
  linear_allocator Allocator;
};

static void HandleSignal(int signum) {
  TerminationRequested = true;
}

static void InitMemory(osx_state *State) {
  memsize MemorySize = 1024*1024*5;
  State->Memory = malloc(MemorySize);
  InitLinearAllocator(&State->Allocator, State->Memory, MemorySize);
}

static void TerminateMemory(osx_state *State) {
  TerminateLinearAllocator(&State->Allocator);
  free(State->Memory);
  State->Memory = NULL;

}

static void ReadNetwork(chunk_list *Events) {
  memsize Length;
  static ui8 ReadBufferBlock[NETWORK_EVENT_MAX_LENGTH];
  static buffer ReadBuffer = {
    .Addr = &ReadBufferBlock,
    .Length = sizeof(ReadBufferBlock)
  };
  while((Length = ReadNetworkEvent(ReadBuffer))) {
    buffer Event = {
      .Addr = ReadBuffer.Addr,
      .Length = Length
    };
    ChunkListWrite(Events, Event);
  }
}

void ExecuteNetCommands(chunk_list *Commands) {
  for(;;) {
    buffer Command = ChunkListRead(Commands);
    if(Command.Length == 0) {
      break;
    }
    network_command_type Type = UnserializeNetworkCommandType(Command);
    switch(Type) {
      case network_command_type_broadcast: {
        broadcast_network_command BroadcastCommand = UnserializeBroadcastNetworkCommand(Command);
        NetworkBroadcast(BroadcastCommand.ClientIDs, BroadcastCommand.ClientIDCount, BroadcastCommand.Message);
        break;
      }
      case network_command_type_shutdown: {
        ShutdownNetwork();
        break;
      }
      default:
        InvalidCodePath;
    }
  }
  ResetChunkList(Commands);
}

int main() {
  osx_state State;

  InitMemory(&State);

  {
    buffer Buffer;
    Buffer.Length = NETWORK_COMMAND_MAX_LENGTH*100;
    Buffer.Addr = LinearAllocate(&State.Allocator, Buffer.Length);
    InitChunkList(&State.NetCommandList, Buffer);
  }

  {
    buffer Buffer;
    Buffer.Length = NETWORK_EVENT_MAX_LENGTH*100;
    Buffer.Addr = LinearAllocate(&State.Allocator, Buffer.Length);
    InitChunkList(&State.NetEventList, Buffer);
  }

  {
    buffer *B = &State.ServerMemory;
    B->Length = 1024*1024;
    B->Addr = LinearAllocate(&State.Allocator, B->Length);
  }
  InitServer(State.ServerMemory);

  InitNetwork();
  {
    int Result = pthread_create(&State.NetworkThread, 0, RunNetwork, 0);
    Assert(Result == 0);
  }

  TerminationRequested = false;
  signal(SIGINT, HandleSignal);
  State.Running = true;
  printf("Listening...\n");
  while(State.Running) {
    ReadNetwork(&State.NetEventList);
    UpdateServer(
      TerminationRequested,
      &State.NetEventList,
      &State.NetCommandList,
      &State.Running,
      State.ServerMemory
    );
    ExecuteNetCommands(&State.NetCommandList);
    ResetChunkList(&State.NetEventList);
  }

  {
    int Result = pthread_join(State.NetworkThread, 0);
    Assert(Result == 0);
  }

  TerminateChunkList(&State.NetEventList);
  TerminateChunkList(&State.NetCommandList);
  TerminateNetwork();
  TerminateMemory(&State);
  printf("Gracefully terminated.\n");
  return 0;
}
