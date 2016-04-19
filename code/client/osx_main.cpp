#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include "lib/assert.h"
#include "common/network_messages.h"
#include "common/memory.h"
#include "network_commands.h"
#include "network_events.h"
#include "client.h"
#include "posix_network.h"

static bool DisconnectRequested;

struct osx_state {
  void *Memory;
  linear_allocator Allocator;
  client_state ClientState;
  chunk_list NetworkCommandList;
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

void FlushNetworkCommands(posix_network_context *Context, chunk_list *Cmds) {
  static ui8 ReadBufferBlock[NETWORK_COMMAND_MAX_LENGTH];
  static buffer ReadBuffer = {
    .Addr = ReadBufferBlock,
    .Length = sizeof(ReadBufferBlock)
  };

  // TODO: Unnecessary to actually copy data
  while(memsize Length = ChunkListRead(Cmds, ReadBuffer)) {
    network_command_type Type = UnserializeNetworkCommandType(ReadBuffer);
    buffer Data = { .Addr = ReadBufferBlock, .Length = Length };
    switch(Type) {
      case network_command_type_send: {
        send_network_command Command = UnserializeSendNetworkCommand(Data);
        NetworkSend(Context, Command.Message);
        break;
      }
      case network_command_type_shutdown: {
        ShutdownNetwork(Context);
        break;
      }
      default:
        InvalidCodePath;
    }
  }
  ResetChunkList(Cmds);
}

int main() {
  osx_state State;

  InitMemory(&State);

  {
    buffer Buffer;
    Buffer.Length = NETWORK_COMMAND_MAX_LENGTH*100;
    Buffer.Addr = LinearAllocate(&State.Allocator, Buffer.Length);
    InitChunkList(&State.NetworkCommandList, Buffer);
  }

  InitNetwork(&State.NetworkContext);
  {
    int Result = pthread_create(&State.NetworkThread, 0, RunNetwork, &State.NetworkContext);
    Assert(Result == 0);
  }

  State.ClientState.TEMP_NETWORK_CONTEXT = &State.NetworkContext;
  InitClient(&State.ClientState);

  signal(SIGINT, HandleSigint);
  while(State.ClientState.Running) {
    // Gather input
    State.ClientState.DisconnectRequested = DisconnectRequested;
    UpdateClient(&State.NetworkCommandList, &State.ClientState);
    FlushNetworkCommands(&State.NetworkContext, &State.NetworkCommandList);
    // Render();
  }

  {
    printf("Waiting for thread join...\n");
    int Result = pthread_join(State.NetworkThread, 0);
    Assert(Result == 0);
  }

  TerminateChunkList(&State.NetworkCommandList);
  TerminateClient(&State.ClientState);
  TerminateNetwork(&State.NetworkContext);
  TerminateMemory(&State);
  printf("Gracefully terminated.\n");
  return 0;
}
