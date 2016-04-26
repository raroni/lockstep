#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include "lib/assert.h"
#include "lib/min_max.h"
#include "lib/chunk_ring_buffer.h"
#include "common/posix_net.h"
#include "common/net_messages.h"
#include "net_events.h"
#include "net_commands.h"
#include "posix_net.h"

static buffer CreateBuffer(memsize Length) {
  buffer B;
  B.Addr = malloc(Length);
  Assert(B.Addr != NULL);
  B.Length = Length;
  return B;
}

static void DestroyBuffer(buffer *B) {
  free(B->Addr);
  B->Addr = NULL;
  B->Length = 0;
}

static void RequestWake(posix_net_context *Context) {
  ui8 X = 1;
  write(Context->WakeWriteFD, &X, 1);
}

static void CheckNewReadFD(posix_net_context *Context, int NewFD) {
  Context->ReadFDMax = MaxInt(Context->ReadFDMax, NewFD);
}

static void RecalcReadFDMax(posix_net_context *Context) {
  Context->ReadFDMax = 0;
  posix_net_client_set_iterator Iterator = CreatePosixNetClientSetIterator(&Context->ClientSet);
  while(AdvancePosixNetClientSetIterator(&Iterator)) {
    CheckNewReadFD(Context, Iterator.Client->FD);
  }
  CheckNewReadFD(Context, Context->WakeReadFD);
  CheckNewReadFD(Context, Context->HostFD);
}

static void InitMemory(posix_net_context *Context) {
  memsize MemorySize = 1024*1024*5;
  Context->Memory = malloc(MemorySize);
  InitLinearAllocator(&Context->Allocator, Context->Memory, MemorySize);
}

static void TerminateMemory(posix_net_context *Context) {
  TerminateLinearAllocator(&Context->Allocator);
  free(Context->Memory);
  Context->Memory = NULL;
}

void InitPosixNet(posix_net_context *Context) {
  InitMemory(Context);

  Context->ReadFDMax = 0;

  {
    int FDs[2];
    pipe(FDs);
    Context->WakeReadFD = FDs[0];
    Context->WakeWriteFD = FDs[1];
    CheckNewReadFD(Context, Context->WakeReadFD);
  }

  {
    memsize CommandBufferLength = 1024*100;
    Context->CommandBufferAddr = malloc(CommandBufferLength);
    buffer CommandBuffer = {
      .Addr = Context->CommandBufferAddr,
      .Length = CommandBufferLength
    };
    InitChunkRingBuffer(&Context->CommandRing, 50, CommandBuffer);
  }

  {
    memsize EventBufferLength = 1024*100;
    Context->EventBufferAddr = malloc(EventBufferLength);
    buffer EventBuffer = {
      .Addr = Context->EventBufferAddr,
      .Length = EventBufferLength
    };
    InitChunkRingBuffer(&Context->EventRing, 50, EventBuffer);
  }

  Context->ReceiveBuffer = CreateBuffer(1024*10);
  Context->EventOutBuffer = CreateBuffer(NET_EVENT_MAX_LENGTH);
  Context->CommandSerializationBuffer = CreateBuffer(NETWORK_COMMAND_MAX_LENGTH);
  Context->CommandReadBuffer = CreateBuffer(NETWORK_COMMAND_MAX_LENGTH);
  Context->IncomingReadBuffer = CreateBuffer(MAX_MESSAGE_LENGTH);

  InitPosixNetClientSet(&Context->ClientSet);

  Context->HostFD = socket(PF_INET, SOCK_STREAM, 0);
  Assert(Context->HostFD != -1);
  CheckNewReadFD(Context, Context->HostFD);
  fcntl(Context->HostFD, F_SETFL, O_NONBLOCK);

  struct sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_len = sizeof(Address);
  Address.sin_family = AF_INET;
  Address.sin_port = htons(4321);
  Address.sin_addr.s_addr = INADDR_ANY;

  int BindResult = bind(Context->HostFD, (struct sockaddr *)&Address, sizeof(Address));
  Assert(BindResult != -1);

  int ListenResult = listen(Context->HostFD, 5);
  Assert(ListenResult == 0);
}

void TerminatePosixNet(posix_net_context *Context) {
  int Result = close(Context->WakeReadFD);
  Assert(Result == 0);
  Result = close(Context->WakeWriteFD);
  Assert(Result == 0);

  Result = close(Context->HostFD);
  Assert(Result == 0);

  DestroyBuffer(&Context->IncomingReadBuffer);
  DestroyBuffer(&Context->CommandReadBuffer);
  DestroyBuffer(&Context->CommandSerializationBuffer);
  DestroyBuffer(&Context->ReceiveBuffer);
  DestroyBuffer(&Context->EventOutBuffer);

  TerminatePosixNetClientSet(&Context->ClientSet);

  TerminateChunkRingBuffer(&Context->CommandRing);
  free(Context->CommandBufferAddr);
  Context->CommandBufferAddr = NULL;

  TerminateChunkRingBuffer(&Context->EventRing);
  free(Context->EventBufferAddr);
  Context->EventBufferAddr = NULL;

  TerminateMemory(Context);
}

void ShutdownPosixNet(posix_net_context *Context) {
  memsize Length = SerializeShutdownNetCommand(Context->CommandSerializationBuffer);
  buffer Command = {
    .Addr = Context->CommandSerializationBuffer.Addr,
    .Length = Length
  };
  ChunkRingBufferWrite(&Context->CommandRing, Command);
  RequestWake(Context);
}

static void ProcessCommands(posix_net_context *Context) {
  memsize Length;
  while((Length = ChunkRingBufferCopyRead(&Context->CommandRing, Context->CommandReadBuffer))) {
    net_command_type Type = UnserializeNetCommandType(Context->CommandReadBuffer);
    buffer Command = {
      .Addr = Context->CommandReadBuffer.Addr,
      .Length = Length
    };
    switch(Type) {
      case net_command_type_broadcast: {
        broadcast_net_command BroadcastCommand = UnserializeBroadcastNetCommand(Command);
        for(memsize I=0; I<BroadcastCommand.ClientIDCount; ++I) {
          posix_net_client *Client = FindClientByID(&Context->ClientSet, BroadcastCommand.ClientIDs[I]);
          if(Client) {
            ssize_t Result = PosixNetSend(Client->FD, BroadcastCommand.Message);
            Assert(Result != -1);
          }
        }
        break;
      }
      case net_command_type_send: {
        send_net_command SendCommand = UnserializeSendNetCommand(Command);
        posix_net_client *Client = FindClientByID(&Context->ClientSet, SendCommand.ClientID);
        if(Client) {
          printf("Sent to client id %zu\n", SendCommand.ClientID);
          ssize_t Result = PosixNetSend(Client->FD, SendCommand.Message);
          Assert(Result != -1);
        }
        break;
      }
      case net_command_type_shutdown: {
        posix_net_client_set_iterator Iterator = CreatePosixNetClientSetIterator(&Context->ClientSet);
        while(AdvancePosixNetClientSetIterator(&Iterator)) {
          int Result = shutdown(Iterator.Client->FD, SHUT_RDWR);
          Assert(Result == 0);
        }
        Context->Mode = net_mode_disconnecting;
        break;
      }
      default:
        InvalidCodePath;
    }
  }
}

memsize ReadPosixNetEvent(posix_net_context *Context, buffer Buffer) {
  return ChunkRingBufferCopyRead(&Context->EventRing, Buffer);
}

void PosixNetBroadcast(posix_net_context *Context, net_client_id *IDs, memsize IDCount, buffer Message) {
  memsize Length = SerializeBroadcastNetCommand(
    IDs,
    IDCount,
    Message,
    Context->CommandSerializationBuffer
  );
  buffer Command = {
    .Addr = Context->CommandSerializationBuffer.Addr,
    .Length = Length
  };
  ChunkRingBufferWrite(&Context->CommandRing, Command);
  RequestWake(Context);
}

void PosixNetSend(posix_net_context *Context, net_client_id ID, buffer Message) {
  memsize Length = SerializeSendNetCommand(ID, Message, Context->CommandSerializationBuffer);
  buffer Command = {
    .Addr = Context->CommandSerializationBuffer.Addr,
    .Length = Length
  };
  ChunkRingBufferWrite(&Context->CommandRing, Command);
  RequestWake(Context);
}

void ProcessIncoming(posix_net_context *Context, posix_net_client *Client) {
  for(;;) {
    buffer Incoming = Context->IncomingReadBuffer;
    Incoming.Length = ByteRingBufferPeek(&Client->InBuffer, Incoming);

    if(Incoming.Length < MinMessageSize) {
      break;
    }
    net_message_type Type = UnserializeNetMessageType(Incoming);
    Assert(ValidateNetMessageType(Type));

    if(!ValidateMessageLength(Incoming, Type)) {
      break;
    }

    memsize MessageLength = 0;
    switch(Type) {
      case net_message_type_reply: {
        // Should unserialize and validate here
        // but I won't because this is just a dummy
        // event that will be deleted soon.
        MessageLength = ReplyNetMessageSize;
        break;
      }
      case net_message_type_order: {
        // TODO: Handle this!
      }
      default:
        InvalidCodePath;
    }

    if(MessageLength == 0) {
      break;
    }
    else {
      Incoming.Length = MessageLength;
      memsize Length = SerializeMessageNetEvent(Client->ID, Incoming, Context->EventOutBuffer);
      buffer Event = {
        .Addr = Context->EventOutBuffer.Addr,
        .Length = Length
      };
      ChunkRingBufferWrite(&Context->EventRing, Event);
      ByteRingBufferReadAdvance(&Client->InBuffer, MessageLength);
    }
  }
}

void* RunPosixNet(void *Data) {
  posix_net_context *Context = (posix_net_context*)Data;
  Context->Mode = net_mode_running;

  while(Context->Mode != net_mode_stopped) {
    fd_set FDSet;
    FD_ZERO(&FDSet);
    {
      posix_net_client_set_iterator Iterator = CreatePosixNetClientSetIterator(&Context->ClientSet);
      while(AdvancePosixNetClientSetIterator(&Iterator)) {
        FD_SET(Iterator.Client->FD, &FDSet);
      }
    }
    FD_SET(Context->HostFD, &FDSet);
    FD_SET(Context->WakeReadFD, &FDSet);

    int SelectResult = select(Context->ReadFDMax+1, &FDSet, NULL, NULL, NULL);
    Assert(SelectResult != -1);

    {
      posix_net_client_set_iterator Iterator = CreatePosixNetClientSetIterator(&Context->ClientSet);
      while(AdvancePosixNetClientSetIterator(&Iterator)) {
        posix_net_client *Client = Iterator.Client;
        if(FD_ISSET(Client->FD, &FDSet)) {
          ssize_t Result = PosixNetReceive(Client->FD, Context->ReceiveBuffer);
          if(Result == 0) {
            int Result = close(Client->FD);
            Assert(Result != -1);
            DestroyClient(&Iterator);
            memsize Length = SerializeDisconnectNetEvent(Client->ID, Context->EventOutBuffer);
            buffer Event = {
              .Addr = Context->EventOutBuffer.Addr,
              .Length = Length
            };
            ChunkRingBufferWrite(&Context->EventRing, Event);
            printf("A client disconnected.\n");
          }
          else {
            buffer Input;
            Input.Addr = Context->ReceiveBuffer.Addr;
            Input.Length = Result;
            printf("Write to %p\n", &Client->InBuffer);
            ByteRingBufferWrite(&Client->InBuffer, Input);
            ProcessIncoming(Context, Client);
          }
        }
        RecalcReadFDMax(Context);
      }
    }

    if(FD_ISSET(Context->WakeReadFD, &FDSet)) {
      ui8 X;
      int Result = read(Context->WakeReadFD, &X, 1);
      Assert(Result != -1);
      ProcessCommands(Context);
    }

    if(
      FD_ISSET(Context->HostFD, &FDSet) &&
      Context->ClientSet.Count != POSIX_NET_CLIENT_SET_MAX &&
      Context->Mode == net_mode_running
    ) {
      int ClientFD = accept(Context->HostFD, NULL, NULL);
      Assert(ClientFD != -1);
      posix_net_client *Client = CreateClient(&Context->ClientSet, ClientFD);
      CheckNewReadFD(Context, ClientFD);
      memsize Length = SerializeConnectNetEvent(Client->ID, Context->EventOutBuffer);
      buffer Event = {
        .Addr = Context->EventOutBuffer.Addr,
        .Length = Length
      };
      ChunkRingBufferWrite(&Context->EventRing, Event);
    }

    if(Context->Mode == net_mode_disconnecting) {
      if(Context->ClientSet.Count == 0) {
        printf("No more clients. Stopping.\n");
        Context->Mode = net_mode_stopped;
      }
    }
  }

  return NULL;
}
