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
  void *Memory;
  const char *Address;
  memory_arena Arena;
  chunk_ring_buffer EventRing;
  chunk_ring_buffer CommandRing;
  byte_ring_buffer IncomingRing;
  buffer CommandReadBuffer;
  buffer ReceiveBuffer;
  buffer IncomingReadBuffer;
  posix_net_state State;
  int WakeReadFD;
  int WakeWriteFD;
  int FDMax;
  int SocketFD;
};

void InitPosixNet(posix_net_context *Context, const char *Address);
void* RunPosixNet(void *Context);
void TerminatePosixNet(posix_net_context *Context);

void PosixNetSend(posix_net_context *Context, buffer Message);
memsize ReadPosixNetEvent(posix_net_context *Context, buffer Buffer);
void ShutdownPosixNet(posix_net_context *Context);
