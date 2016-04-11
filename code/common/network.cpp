#include <netinet/in.h>
#include "lib/assert.h"
#include "network.h"

ssize_t NetworkReceive(int FD, buffer Buffer) {
  Assert(Buffer.Length != 0);
  ssize_t Result = recv(FD, Buffer.Addr, Buffer.Length, 0);
  return Result;
}
