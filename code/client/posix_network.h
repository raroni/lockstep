#pragma once

#include "lib/chunk_ring_buffer.h"
#include "lib/byte_ring_buffer.h"

enum posix_network_state {
  posix_network_state_inactive,
  posix_network_state_connecting,
  posix_network_state_connected,
  posix_network_state_shutting_down,
  posix_network_state_stopped
};

struct posix_network_context {
  void *EventBufferAddr;
  chunk_ring_buffer EventRing;
  void *CommandBufferAddr;
  chunk_ring_buffer CommandRing;
  void *IncomingBufferAddr;
  byte_ring_buffer IncomingRing;
  buffer CommandSerializationBuffer;
  buffer CommandReadBuffer;
  buffer ReceiveBuffer;
  buffer IncomingReadBuffer;
  buffer EventSerializationBuffer;
  posix_network_state State;
  int WakeReadFD;
  int WakeWriteFD;
  int FDMax;
  int SocketFD;
};

void InitNetwork(posix_network_context *Context);
void* RunNetwork(void *Context);
void TerminateNetwork(posix_network_context *Context);

void NetworkSend(posix_network_context *Context, buffer Message);
memsize ReadNetworkEvent(posix_network_context *Context, buffer Buffer);
void ShutdownNetwork(posix_network_context *Context);
