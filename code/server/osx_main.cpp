#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include "lib/assert.h"
#include "lib/chunk_list.h"
#include "common/memory.h"
#include "net_commands.h"
#include "net_events.h"
#include "game.h"
#include "posix_net.h"

static bool TerminationRequested;

struct osx_state {
  posix_net_context NetContext;
  pthread_t NetThread;
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

static void ReadNet(posix_net_context *Context, chunk_list *Events) {
  memsize Length;
  static ui8 ReadBufferBlock[NETWORK_EVENT_MAX_LENGTH];
  static buffer ReadBuffer = {
    .Addr = &ReadBufferBlock,
    .Length = sizeof(ReadBufferBlock)
  };
  while((Length = ReadPosixNetEvent(Context, ReadBuffer))) {
    buffer Event = {
      .Addr = ReadBuffer.Addr,
      .Length = Length
    };
    ChunkListWrite(Events, Event);
  }
}

void ExecuteNetCommands(posix_net_context *Context, chunk_list *Commands) {
  for(;;) {
    buffer Command = ChunkListRead(Commands);
    if(Command.Length == 0) {
      break;
    }
    net_command_type Type = UnserializeNetCommandType(Command);
    switch(Type) {
      case net_command_type_broadcast: {
        broadcast_net_command BroadcastCommand = UnserializeBroadcastNetCommand(Command);
        PosixNetBroadcast(Context, BroadcastCommand.ClientIDs, BroadcastCommand.ClientIDCount, BroadcastCommand.Message);
        break;
      }
      case net_command_type_shutdown: {
        ShutdownPosixNet(Context);
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
  InitGame(State.ServerMemory);

  InitPosixNet(&State.NetContext);
  {
    int Result = pthread_create(&State.NetThread, 0, RunPosixNet, &State.NetContext);
    Assert(Result == 0);
  }

  TerminationRequested = false;
  signal(SIGINT, HandleSignal);
  State.Running = true;
  printf("Listening...\n");
  while(State.Running) {
    ReadNet(&State.NetContext, &State.NetEventList);
    UpdateGame(
      TerminationRequested,
      &State.NetEventList,
      &State.NetCommandList,
      &State.Running,
      State.ServerMemory
    );
    ExecuteNetCommands(&State.NetContext, &State.NetCommandList);
    ResetChunkList(&State.NetEventList);
  }

  {
    int Result = pthread_join(State.NetThread, 0);
    Assert(Result == 0);
  }

  TerminateChunkList(&State.NetEventList);
  TerminateChunkList(&State.NetCommandList);
  TerminatePosixNet(&State.NetContext);
  TerminateMemory(&State);
  printf("Gracefully terminated.\n");
  return 0;
}
