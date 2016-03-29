#include <netinet/in.h>
#include "assert.h"
#include "network.h"

void InitNetworkBuffer(network_buffer *Buffer, void *Data, size_t Capacity) {
  Buffer->Data = Data;
  Buffer->Capacity = Capacity;
  Buffer->Length = 0;
}

void TerminateNetworkBuffer(network_buffer *Buffer) {
  Buffer->Data = NULL;
  Buffer->Capacity = 0;
  Buffer->Length = 0;
}

size_t NetworkReceive(int FD, network_buffer *Buffer) {
  Assert(Buffer->Capacity != 0);
  // TODO: Loop until you have all
  ssize_t Result = recv(FD, Buffer->Data, Buffer->Capacity, 0);
  return Result;
}
