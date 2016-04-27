#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib/def.h"
#include "lib/min_max.h"
#include "lib/assert.h"
#include "common/posix_net.h"
#include "common/net_messages.h"
#include "net_events.h"
#include "net_commands.h"
#include "posix_net.h"

enum errno_code {
  errno_code_interrupted_system_call = 4,
  errno_code_in_progress = 36
};

static void RequestWake(posix_net_context *Context) {
  ui8 X = 1;
  write(Context->WakeWriteFD, &X, 1);
}

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

void InitPosixNet(posix_net_context *Context) {
  {
    int SocketFD = socket(PF_INET, SOCK_STREAM, 0);
    Assert(SocketFD != -1);
    Context->SocketFD = SocketFD;
  }

  {
    int Result = fcntl(Context->SocketFD, F_SETFL, O_NONBLOCK);
    Assert(Result != -1);
  }

  {
    int FDs[2];
    int Result = pipe(FDs);
    Assert(Result != -1);
    Context->WakeReadFD = FDs[0];
    Context->WakeWriteFD = FDs[1];
  }

  Context->FDMax = MaxInt(Context->WakeReadFD, Context->SocketFD);

  {
    memsize Length = 1024*100;
    Context->EventBufferAddr = malloc(Length);
    buffer Buffer = {
      .Addr = Context->EventBufferAddr,
      .Length = Length
    };
    InitChunkRingBuffer(&Context->EventRing, 50, Buffer);
  }

  {
    memsize Length = 1024*100;
    Context->CommandBufferAddr = malloc(Length);
    buffer Buffer = {
      .Addr = Context->CommandBufferAddr,
      .Length = Length
    };
    InitChunkRingBuffer(&Context->CommandRing, 50, Buffer);
  }

  {
    memsize Length = 1024*100;
    Context->IncomingBufferAddr = malloc(Length);
    buffer Buffer = {
      .Addr = Context->IncomingBufferAddr,
      .Length = Length
    };
    InitByteRingBuffer(&Context->IncomingRing, Buffer);
  }

  Context->CommandSerializationBuffer = CreateBuffer(NETWORK_COMMAND_MAX_LENGTH);
  Context->CommandReadBuffer = CreateBuffer(NETWORK_COMMAND_MAX_LENGTH);
  Context->ReceiveBuffer = CreateBuffer(1024*10);
  Context->EventSerializationBuffer = CreateBuffer(NET_EVENT_MAX_LENGTH);
  Context->IncomingReadBuffer = CreateBuffer(NET_MESSAGE_MAX_LENGTH);

  Context->State = posix_net_state_inactive;
}

void Connect(posix_net_context *Context) {
  struct sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_len = sizeof(Address);
  Address.sin_family = AF_INET;
  Address.sin_port = htons(4321);
  Address.sin_addr.s_addr = 0;

  int ConnectResult = connect(Context->SocketFD, (struct sockaddr *)&Address, sizeof(Address));
  Assert(ConnectResult != -1 || errno == errno_code_in_progress);

  printf("Connecting...\n");

  Context->State = posix_net_state_connecting;
}

memsize ReadPosixNetEvent(posix_net_context *Context, buffer Buffer) {
  return ChunkRingBufferCopyRead(&Context->EventRing, Buffer);
}

void ProcessCommands(posix_net_context *Context) {
  memsize Length;
  while((Length = ChunkRingBufferCopyRead(&Context->CommandRing, Context->CommandReadBuffer))) {
    net_command_type Type = UnserializeNetCommandType(Context->CommandReadBuffer);
    buffer Command = {
      .Addr = Context->CommandReadBuffer.Addr,
      .Length = Length
    };
    switch(Type) {
      case net_command_type_shutdown: {
        if(Context->State != posix_net_state_shutting_down) {
          printf("Shutting down...\n");
          int Result = shutdown(Context->SocketFD, SHUT_RDWR);
          Assert(Result == 0);
          Context->State = posix_net_state_shutting_down;
        }
        break;
      }
      case net_command_type_send: {
        if(Context->State == posix_net_state_connected) {
          send_net_command SendCommand = UnserializeSendNetCommand(Command);
          buffer Message = SendCommand.Message;
          printf("Sending message of size %zu!\n", Message.Length);
          PosixNetSendPacket(Context->SocketFD, Message);
        }
        break;
      }
    }
  }
}

void ProcessIncoming(posix_net_context *Context) {
  for(;;) {
    buffer Incoming = Context->IncomingReadBuffer;
    Incoming.Length = ByteRingBufferPeek(&Context->IncomingRing, Incoming);

    buffer Message = PosixExtractPacketMessage(Incoming);
    if(Message.Length == 0) {
      break;
    }

    net_message_type Type = UnserializeNetMessageType(Message);
    Assert(ValidateNetMessageType(Type));

    switch(Type) {
      case net_message_type_start: {
        start_net_message StartMessage = UnserializeStartNetMessage(Message);
        Assert(ValidateStartNetMessage(StartMessage));
        break;
      }
      case net_message_type_order_list: {
        order_list_net_message ListMessage = UnserializeOrderListNetMessage(Message);
        Assert(ValidateOrderListNetMessage(ListMessage));
        break;
      }
      default:
        InvalidCodePath;
    }

    memsize Length = SerializeMessageNetEvent(Message, Context->EventSerializationBuffer);
    buffer Event = {
      .Addr = Context->EventSerializationBuffer.Addr,
      .Length = Length
    };
    ChunkRingBufferWrite(&Context->EventRing, Event);
    ByteRingBufferReadAdvance(&Context->IncomingRing, POSIX_PACKET_HEADER_SIZE + Message.Length);
  }
}

void* RunPosixNet(void *VoidContext) {
  posix_net_context *Context = (posix_net_context*)VoidContext;
  Connect(Context);

  while(Context->State != posix_net_state_stopped) {
    fd_set FDSet;
    FD_ZERO(&FDSet);
    FD_SET(Context->SocketFD, &FDSet);
    FD_SET(Context->WakeReadFD, &FDSet);

    int SelectResult = select(Context->FDMax+1, &FDSet, NULL, NULL, NULL);
    Assert(SelectResult != -1);

    if(FD_ISSET(Context->WakeReadFD, &FDSet)) {
      ui8 X;
      int Result = read(Context->WakeReadFD, &X, 1);
      Assert(Result != -1);
      ProcessCommands(Context);
    }

    if(FD_ISSET(Context->SocketFD, &FDSet)) {
      if(Context->State == posix_net_state_connecting) {
        int OptionValue;
        socklen_t OptionLength = sizeof(OptionValue);
        int Result = getsockopt(Context->SocketFD, SOL_SOCKET, SO_ERROR, &OptionValue, &OptionLength);
        Assert(Result == 0);
        if(OptionValue == 0) {
          Context->State = posix_net_state_connected;

          memsize Length = SerializeConnectionEstablishedNetEvent(Context->EventSerializationBuffer);
          buffer Event = {
            .Addr = Context->EventSerializationBuffer.Addr,
            .Length = Length
          };
          ChunkRingBufferWrite(&Context->EventRing, Event);

          printf("Connected.\n");
        }
        else {
          printf("Connection failed.\n");

          memsize Length = SerializeConnectionFailedNetEvent(Context->EventSerializationBuffer);
          buffer Event = {
            .Addr = Context->EventSerializationBuffer.Addr,
            .Length = Length
          };
          ChunkRingBufferWrite(&Context->EventRing, Event);

          Context->State = posix_net_state_stopped;
        }
      }
      else {
        ssize_t ReceivedCount = PosixNetReceive(Context->SocketFD, Context->ReceiveBuffer);
        if(ReceivedCount == 0) {
          printf("Disconnected.\n");

          ByteRingBufferReset(&Context->IncomingRing);

          memsize Length = SerializeConnectionLostNetEvent(Context->EventSerializationBuffer);
          buffer Event = {
            .Addr = Context->EventSerializationBuffer.Addr,
            .Length = Length
          };
          ChunkRingBufferWrite(&Context->EventRing, Event);

          Context->State = posix_net_state_stopped;
          continue;
        }
        else if(Context->State == posix_net_state_connected) {
          buffer Incoming = {
            .Addr = Context->ReceiveBuffer.Addr,
            .Length = (memsize)ReceivedCount
          };
          ByteRingBufferWrite(&Context->IncomingRing, Incoming);
          ProcessIncoming(Context);
        }
      }
    }
  }

  printf("Net thread done.\n");

  return NULL;
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

void PosixNetSend(posix_net_context *Context, buffer Message) {
  memsize Length = SerializeSendNetCommand(Context->CommandSerializationBuffer, Message);
  buffer Command = {
    .Addr = Context->CommandSerializationBuffer.Addr,
    .Length = Length
  };
  ChunkRingBufferWrite(&Context->CommandRing, Command);
  RequestWake(Context);
}

void TerminatePosixNet(posix_net_context *Context) {
  int Result = close(Context->SocketFD);
  Assert(Result == 0);

  Result = close(Context->WakeReadFD);
  Assert(Result == 0);
  Result = close(Context->WakeWriteFD);
  Assert(Result == 0);

  DestroyBuffer(&Context->IncomingReadBuffer);
  DestroyBuffer(&Context->CommandSerializationBuffer);
  DestroyBuffer(&Context->CommandReadBuffer);
  DestroyBuffer(&Context->ReceiveBuffer);
  DestroyBuffer(&Context->EventSerializationBuffer);

  TerminateChunkRingBuffer(&Context->EventRing);
  free(Context->EventBufferAddr);
  Context->EventBufferAddr = NULL;

  TerminateChunkRingBuffer(&Context->CommandRing);
  free(Context->CommandBufferAddr);
  Context->CommandBufferAddr = NULL;

  TerminateByteRingBuffer(&Context->IncomingRing);
  free(Context->IncomingBufferAddr);
  Context->IncomingBufferAddr = NULL;

  Context->State = posix_net_state_inactive;
}
