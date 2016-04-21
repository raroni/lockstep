#pragma once

#include "lib/chunk_ring_buffer.h"
#include "lib/byte_ring_buffer.h"

enum posix_net_state {
  posix_net_state_inactive,
  posix_net_state_connecting,
  posix_net_state_connected,
  posix_net_state_shutting_down,
  posix_net_state_stopped
};

struct posix_net_context {
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
  posix_net_state State;
  int WakeReadFD;
  int WakeWriteFD;
  int FDMax;
  int SocketFD;
};

void InitPosixNet(posix_net_context *Context);
void* RunPosixNet(void *Context);
void TerminatePosixNet(posix_net_context *Context);

void PosixNetSend(posix_net_context *Context, buffer Message);
memsize ReadPosixNetEvent(posix_net_context *Context, buffer Buffer);
void ShutdownPosixNet(posix_net_context *Context);
