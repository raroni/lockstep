#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include "lib/assert.h"
#include "lib/min_max.h"
#include "lib/chunk_ring_buffer.h"
#include "common/network.h"
#include "common/network_messages.h"
#include "network_events.h"
#include "network_commands.h"
#include "network.h"

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

static void RequestWake(network_context *Context) {
  ui8 X = 1;
  write(Context->WakeWriteFD, &X, 1);
}

static void CheckNewReadFD(network_context *Context, int NewFD) {
  Context->ReadFDMax = MaxInt(Context->ReadFDMax, NewFD);
}

static void RecalcReadFDMax(network_context *Context) {
  Context->ReadFDMax = 0;
  client_set_iterator Iterator = CreateClientSetIterator(&Context->ClientSet);
  while(AdvanceClientSetIterator(&Iterator)) {
    CheckNewReadFD(Context, Iterator.Client->FD);
  }
  CheckNewReadFD(Context, Context->WakeReadFD);
  CheckNewReadFD(Context, Context->HostFD);
}

void InitNetwork(network_context *Context) {
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
  Context->EventOutBuffer = CreateBuffer(NETWORK_EVENT_MAX_LENGTH);
  Context->CommandSerializationBuffer = CreateBuffer(NETWORK_COMMAND_MAX_LENGTH);
  Context->CommandReadBuffer = CreateBuffer(NETWORK_COMMAND_MAX_LENGTH);
  Context->IncomingReadBuffer = CreateBuffer(MAX_MESSAGE_LENGTH);

  InitClientSet(&Context->ClientSet);

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

void TerminateNetwork(network_context *Context) {
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

  TerminateClientSet(&Context->ClientSet);

  TerminateChunkRingBuffer(&Context->CommandRing);
  free(Context->CommandBufferAddr);
  Context->CommandBufferAddr = NULL;

  TerminateChunkRingBuffer(&Context->EventRing);
  free(Context->EventBufferAddr);
  Context->EventBufferAddr = NULL;
}

void ShutdownNetwork(network_context *Context) {
  memsize Length = SerializeShutdownNetworkCommand(Context->CommandSerializationBuffer);
  buffer Command = {
    .Addr = Context->CommandSerializationBuffer.Addr,
    .Length = Length
  };
  ChunkRingBufferWrite(&Context->CommandRing, Command);
  RequestWake(Context);
}

static void ProcessCommands(network_context *Context) {
  memsize Length;
  while((Length = ChunkRingBufferRead(&Context->CommandRing, Context->CommandReadBuffer))) {
    network_command_type Type = UnserializeNetworkCommandType(Context->CommandReadBuffer);
    buffer Command = {
      .Addr = Context->CommandReadBuffer.Addr,
      .Length = Length
    };
    switch(Type) {
      case network_command_type_shutdown: {
        client_set_iterator Iterator = CreateClientSetIterator(&Context->ClientSet);
        while(AdvanceClientSetIterator(&Iterator)) {
          int Result = shutdown(Iterator.Client->FD, SHUT_RDWR);
          Assert(Result == 0);
        }
        Context->Mode = network_mode_disconnecting;
        break;
      }
      case network_command_type_broadcast: {
        broadcast_network_command BroadcastCommand = UnserializeBroadcastNetworkCommand(Command);
        for(memsize I=0; I<BroadcastCommand.ClientIDCount; ++I) {
          client *Client = FindClientByID(&Context->ClientSet, BroadcastCommand.ClientIDs[I]);
          if(Client) {
            printf("Broadcasted to client id %zu\n", BroadcastCommand.ClientIDs[I]);
            ssize_t Result = NetworkSend(Client->FD, BroadcastCommand.Message);
            Assert(Result != -1);
          }
        }
        break;
      }
      default:
        InvalidCodePath;
    }
  }
}

memsize ReadNetworkEvent(network_context *Context, buffer Buffer) {
  return ChunkRingBufferRead(&Context->EventRing, Buffer);
}

void NetworkBroadcast(network_context *Context, client_id *IDs, memsize IDCount, buffer Message) {
  memsize Length = SerializeBroadcastNetworkCommand(
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

void ProcessIncoming(network_context *Context, client *Client) {
  for(;;) {
    buffer Incoming = Context->IncomingReadBuffer;
    Incoming.Length = ByteRingBufferPeek(&Client->InBuffer, Incoming);
    network_message_type Type;
    bool Result = UnserializeNetworkMessageType(Incoming, &Type);
    if(!Result) {
      break;
    }

    memsize ConsumedBytesCount = 0;
    switch(Type) {
      case network_message_type_reply: {
        memsize Length = SerializeReplyNetworkEvent(Client->ID, Context->EventOutBuffer);
        buffer Event = {
          .Addr = Context->EventOutBuffer.Addr,
          .Length = Length
        };
        ChunkRingBufferWrite(&Context->EventRing, Event);
        ConsumedBytesCount = ReplyNetworkMesageSize;
        break;
      }
      default:
        InvalidCodePath;
    }

    if(ConsumedBytesCount == 0) {
      break;
    }
    else {
      ByteRingBufferReadAdvance(&Client->InBuffer, ConsumedBytesCount);
    }
  }
}

void* RunNetwork(void *Data) {
  network_context *Context = (network_context*)Data;
  Context->Mode = network_mode_running;

  while(Context->Mode != network_mode_stopped) {
    fd_set FDSet;
    FD_ZERO(&FDSet);
    {
      client_set_iterator Iterator = CreateClientSetIterator(&Context->ClientSet);
      while(AdvanceClientSetIterator(&Iterator)) {
        FD_SET(Iterator.Client->FD, &FDSet);
      }
    }
    FD_SET(Context->HostFD, &FDSet);
    FD_SET(Context->WakeReadFD, &FDSet);

    int SelectResult = select(Context->ReadFDMax+1, &FDSet, NULL, NULL, NULL);
    Assert(SelectResult != -1);

    {
      client_set_iterator Iterator = CreateClientSetIterator(&Context->ClientSet);
      while(AdvanceClientSetIterator(&Iterator)) {
        client *Client = Iterator.Client;
        if(FD_ISSET(Client->FD, &FDSet)) {
          ssize_t Result = NetworkReceive(Client->FD, Context->ReceiveBuffer);
          if(Result == 0) {
            int Result = close(Client->FD);
            Assert(Result != -1);
            DestroyClient(&Iterator);
            memsize Length = SerializeDisconnectNetworkEvent(Client->ID, Context->EventOutBuffer);
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
      Context->ClientSet.Count != CLIENT_SET_MAX &&
      Context->Mode == network_mode_running
    ) {
      int ClientFD = accept(Context->HostFD, NULL, NULL);
      Assert(ClientFD != -1);
      client *Client = CreateClient(&Context->ClientSet, ClientFD);
      CheckNewReadFD(Context, ClientFD);
      memsize Length = SerializeConnectNetworkEvent(Client->ID, Context->EventOutBuffer);
      buffer Event = {
        .Addr = Context->EventOutBuffer.Addr,
        .Length = Length
      };
      ChunkRingBufferWrite(&Context->EventRing, Event);
    }

    if(Context->Mode == network_mode_disconnecting) {
      if(Context->ClientSet.Count == 0) {
        printf("No more clients. Stopping.\n");
        Context->Mode = network_mode_stopped;
      }
    }
  }

  return NULL;
}
