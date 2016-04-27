#include <netinet/in.h>
#include "lib/assert.h"
#include "posix_net.h"

ssize_t PosixNetReceive(int FD, buffer Buffer) {
  Assert(Buffer.Length != 0);
  ssize_t Result = recv(FD, Buffer.Addr, Buffer.Length, 0);
  return Result;
}

buffer PosixExtractPacketMessage(buffer Incoming) {
  buffer Message;
  Message.Addr = NULL;
  Message.Length = 0;

  if(Incoming.Length > sizeof(ui16)) {
    ui16 MessageLength = *(ui16*)Incoming.Addr;
    if(Incoming.Length >= MessageLength + POSIX_PACKET_HEADER_SIZE) {
      Message.Addr = (ui8*)Incoming.Addr + POSIX_PACKET_HEADER_SIZE;
      Message.Length = MessageLength;
    }
  }

  return Message;
}

void PosixNetSendPacket(int FD, buffer Message) {
  Assert(UINT16_MAX >= Message.Length);
  ui16 Length = Message.Length;
  ssize_t Result = send(FD, &Length, sizeof(Length), 0);
  Assert(Result == sizeof(Length));

  Result = send(FD, Message.Addr, Message.Length, 0);
  Assert(Result == Message.Length);
}
