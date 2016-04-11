#include <netinet/in.h>
#include "lib/def.h"
#include "lib/assert.h"
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

ssize_t NetworkReceive(int FD, network_buffer *Buffer) {
  Assert(Buffer->Capacity != 0);
  void *Destination = ((ui8*)Buffer->Data) + Buffer->Length;
  ssize_t Capacity = Buffer->Capacity - Buffer->Length;
  ssize_t Result = recv(FD, Destination, Capacity, 0);
  return Result;
}
