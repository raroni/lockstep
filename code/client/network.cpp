#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib/def.h"
#include "lib/min_max.h"
#include "lib/assert.h"
#include "lib/chunk_ring_buffer.h"
#include "lib/byte_ring_buffer.h"
#include "common/network.h"
#include "common/network_messages.h"
#include "network.h"
#include "network_events.h"
#include "network_commands.h"

enum errno_code {
  errno_code_interrupted_system_call = 4,
  errno_code_in_progress = 36
};

enum main_state {
  main_state_inactive,
  main_state_connecting,
  main_state_connected,
  main_state_shutting_down,
  main_state_stopped
};

static int SocketFD;
static main_state MainState;
static int WakeReadFD;
static int WakeWriteFD;
static int FDMax;

static ui8 CommandSerializationBufferBlock[COMMAND_MAX_LENGTH];
static buffer CommandSerializationBuffer = {
  .Addr = &CommandSerializationBufferBlock,
  .Length = sizeof(CommandSerializationBufferBlock)
};

static ui8 ReceiveBufferBlock[1024*10];
static buffer ReceiveBuffer = {
  .Addr = &ReceiveBufferBlock,
  .Length = sizeof(ReceiveBufferBlock)
};

static ui8 EventSerializationBufferBlock[NETWORK_EVENT_MAX_LENGTH];
static buffer EventSerializationBuffer = {
  .Addr = &EventSerializationBufferBlock,
  .Length = sizeof(EventSerializationBufferBlock)
};
static void *EventBufferAddr;
static chunk_ring_buffer EventRing;
static void *CommandBufferAddr;
static chunk_ring_buffer CommandRing;
static void *IncomingBufferAddr;
static byte_ring_buffer IncomingRing;

static void RequestWake() {
  ui8 X = 1;
  write(WakeWriteFD, &X, 1);
}

void InitNetwork() {
  SocketFD = socket(PF_INET, SOCK_STREAM, 0);
  Assert(SocketFD != -1);
  int Result = fcntl(SocketFD, F_SETFL, O_NONBLOCK);
  Assert(Result != -1);

  {
    int FDs[2];
    pipe(FDs);
    WakeReadFD = FDs[0];
    WakeWriteFD = FDs[1];
  }

  FDMax = MaxInt(WakeReadFD, SocketFD);

  {
    memsize Length = 1024*100;
    EventBufferAddr = malloc(Length);
    buffer Buffer = {
      .Addr = EventBufferAddr,
      .Length = Length
    };
    InitChunkRingBuffer(&EventRing, 50, Buffer);
  }

  {
    memsize Length = 1024*100;
    CommandBufferAddr = malloc(Length);
    buffer Buffer = {
      .Addr = CommandBufferAddr,
      .Length = Length
    };
    InitChunkRingBuffer(&CommandRing, 50, Buffer);
  }

  {
    memsize Length = 1024*100;
    IncomingBufferAddr = malloc(Length);
    buffer Buffer = {
      .Addr = IncomingBufferAddr,
      .Length = Length
    };
    InitByteRingBuffer(&IncomingRing, Buffer);
  }

  MainState = main_state_inactive;
}

void Connect() {
  struct sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_len = sizeof(Address);
  Address.sin_family = AF_INET;
  Address.sin_port = htons(4321);
  Address.sin_addr.s_addr = 0;

  int ConnectResult = connect(SocketFD, (struct sockaddr *)&Address, sizeof(Address));
  Assert(ConnectResult != -1 || errno == errno_code_in_progress);

  printf("Connecting...\n");

  MainState = main_state_connecting;
}

memsize ReadNetworkEvent(buffer Buffer) {
  return ChunkRingBufferRead(&EventRing, Buffer);
}

void ProcessCommands(main_state *MainState) {
  memsize Length;
  static ui8 BufferStorage[COMMAND_MAX_LENGTH];
  static buffer Buffer = { .Addr = BufferStorage, .Length = COMMAND_MAX_LENGTH };
  while((Length = ChunkRingBufferRead(&CommandRing, Buffer))) {
    network_command_type Type = UnserializeNetworkCommandType(Buffer);
    buffer Command = {
      .Addr = Buffer.Addr,
      .Length = Length
    };
    switch(Type) {
      case network_command_type_shutdown: {
        if(*MainState != main_state_shutting_down) {
          printf("Shutting down...\n");
          int Result = shutdown(SocketFD, SHUT_RDWR);
          Assert(Result == 0);
          *MainState = main_state_shutting_down;
        }
        break;
      }
      case network_command_type_send: {
        if(*MainState == main_state_connected) {
          send_network_command SendCommand = UnserializeSendNetworkCommand(Command);
          buffer Message = SendCommand.Message;
          printf("Sending message of size %zu!\n", Message.Length);
          NetworkSend(SocketFD, Message);
        }
        break;
      }
    }
  }
}

void ProcessIncoming() {
  static ui8 IncomingBlock[MAX_MESSAGE_LENGTH];
  buffer Incoming = { .Addr = &IncomingBlock, .Length = sizeof(IncomingBlock) };

  for(;;) {
    Incoming.Length = ByteRingBufferPeek(&IncomingRing, Incoming);
    network_message_type Type;
    bool Result = UnserializeNetworkMessageType(Incoming, &Type);
    if(!Result) {
      break;
    }

    memsize ConsumedBytesCount = 0;
    switch(Type) {
      case network_message_type_start: {
        memsize Length = SerializeStartNetworkEvent(EventSerializationBuffer);
        buffer Event = {
          .Addr = EventSerializationBuffer.Addr,
          .Length = Length
        };
        ChunkRingBufferWrite(&EventRing, Event);
        ConsumedBytesCount = StartNetworkMesageSize;
        break;
      }
      default:
        InvalidCodePath;
    }

    if(ConsumedBytesCount == 0) {
      break;
    }
    else {
      ByteRingBufferReadAdvance(&IncomingRing, ConsumedBytesCount);
    }
  }
}

void* RunNetwork(void *Data) {
  Connect();

  while(MainState != main_state_stopped) {
    fd_set FDSet;
    FD_ZERO(&FDSet);
    FD_SET(SocketFD, &FDSet);
    FD_SET(WakeReadFD, &FDSet);

    int SelectResult = select(FDMax+1, &FDSet, NULL, NULL, NULL);
    Assert(SelectResult != -1);

    if(FD_ISSET(WakeReadFD, &FDSet)) {
      ui8 X;
      int Result = read(WakeReadFD, &X, 1);
      Assert(Result != -1);
      ProcessCommands(&MainState);
    }

    if(FD_ISSET(SocketFD, &FDSet)) {
      if(MainState == main_state_connecting) {
        int OptionValue;
        socklen_t OptionLength = sizeof(OptionValue);
        int Result = getsockopt(SocketFD, SOL_SOCKET, SO_ERROR, &OptionValue, &OptionLength);
        Assert(Result == 0);
        if(OptionValue == 0) {
          MainState = main_state_connected;

          memsize Length = SerializeConnectionEstablishedNetworkEvent(EventSerializationBuffer);
          buffer Event = {
            .Addr = EventSerializationBuffer.Addr,
            .Length = Length
          };
          ChunkRingBufferWrite(&EventRing, Event);

          printf("Connected.\n");
        }
        else {
          printf("Connection failed.\n");

          memsize Length = SerializeConnectionFailedNetworkEvent(EventSerializationBuffer);
          buffer Event = {
            .Addr = EventSerializationBuffer.Addr,
            .Length = Length
          };
          ChunkRingBufferWrite(&EventRing, Event);

          MainState = main_state_stopped;
        }
      }
      else {
        ssize_t ReceivedCount = NetworkReceive(SocketFD, ReceiveBuffer);
        if(ReceivedCount == 0) {
          printf("Disconnected.\n");

          ByteRingBufferReset(&IncomingRing);

          memsize Length = SerializeConnectionLostNetworkEvent(EventSerializationBuffer);
          buffer Event = {
            .Addr = EventSerializationBuffer.Addr,
            .Length = Length
          };
          ChunkRingBufferWrite(&EventRing, Event);

          MainState = main_state_stopped;
          continue;
        }
        else if(MainState == main_state_connected) {
          buffer Incoming = {
            .Addr = ReceiveBuffer.Addr,
            .Length = (memsize)ReceivedCount
          };
          ByteRingBufferWrite(&IncomingRing, Incoming);
          printf("Got something of length: %zd, as char %u\n", (int)ReceivedCount, *(ui8*)ReceiveBuffer.Addr);
          ProcessIncoming();
        }
      }
    }
  }

  printf("Network thread done.\n");

  return NULL;
}

void ShutdownNetwork() {
  memsize Length = SerializeShutdownNetworkCommand(CommandSerializationBuffer);
  buffer Command = {
    .Addr = CommandSerializationBuffer.Addr,
    .Length = Length
  };
  ChunkRingBufferWrite(&CommandRing, Command);
  RequestWake();
}

void NetworkSend(buffer Message) {
  memsize Length = SerializeSendNetworkCommand(CommandSerializationBuffer, Message);
  buffer Command = {
    .Addr = CommandSerializationBuffer.Addr,
    .Length = Length
  };
  ChunkRingBufferWrite(&CommandRing, Command);
  RequestWake();
}

void TerminateNetwork() {
  int Result = close(SocketFD);
  Assert(Result == 0);

  Result = close(WakeReadFD);
  Assert(Result == 0);
  Result = close(WakeWriteFD);
  Assert(Result == 0);

  TerminateChunkRingBuffer(&EventRing);
  free(EventBufferAddr);
  EventBufferAddr = NULL;

  TerminateChunkRingBuffer(&CommandRing);
  free(CommandBufferAddr);
  CommandBufferAddr = NULL;

  TerminateByteRingBuffer(&IncomingRing);
  free(IncomingBufferAddr);
  IncomingBufferAddr = NULL;

  MainState = main_state_inactive;
}
