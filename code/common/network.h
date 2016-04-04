#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>

struct network_buffer {
  void *Data;
  size_t Length;
  size_t Capacity;
};

void InitNetworkBuffer(network_buffer *Buffer, void *Data, size_t Capacity);
void TerminateNetworkBuffer(network_buffer *Buffer);

ssize_t NetworkReceive(int FD, network_buffer *Buffer);

#endif
