#include <netinet/in.h>
#include "lib/assert.h"
#include "posix_net.h"

ssize_t PosixNetReceive(int FD, buffer Buffer) {
  Assert(Buffer.Length != 0);
  ssize_t Result = recv(FD, Buffer.Addr, Buffer.Length, 0);
  return Result;
}

ssize_t PosixNetSend(int FD, buffer Buffer) {
  ssize_t Result = send(FD, Buffer.Addr, Buffer.Length, 0);
  return Result;
}
