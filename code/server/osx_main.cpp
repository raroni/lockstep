#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include "lib/assert.h"
#include "lib/chunk_list.h"
#include "common/memory.h"
#include "common/posix_time.h"
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
      case net_command_type_send: {
        send_net_command SendCommand = UnserializeSendNetCommand(Command);
        PosixNetSend(Context, SendCommand.ClientID, SendCommand.Message);
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
  ui64 GameStartTime = GetTime();
  while(State.Running) {
    ui64 GameDuration = GetTime() - GameStartTime;
    ui64 Delay = 0;
    ReadNet(&State.NetContext, &State.NetEventList);
    UpdateGame(
      GameDuration,
      &Delay,
      TerminationRequested,
      &State.NetEventList,
      &State.NetCommandList,
      &State.Running,
      State.ServerMemory
    );
    ExecuteNetCommands(&State.NetContext, &State.NetCommandList);
    ResetChunkList(&State.NetEventList);

    ui64 ConservativeDelay = Delay/2;
    if(ConservativeDelay > 200) {
      usleep(ConservativeDelay);
    }
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




// clude <unistd.h>
// clude <sys/time.h>
// clude "SysTime/SysTime.h"

// namespace SysTime {
//   USecond64 get() {
//     struct timeval tv;
//     gettimeofday(&tv, NULL);
//     return (tv.tv_sec*1000000+tv.tv_usec);
//   }

//   void sleep(USecond64 duration) {
//     usleep(static_cast<useconds_t>(duration));
//   }
// }
