#pragma once

#include "lib/def.h"
#include "lib/chunk_ring_buffer.h"
#include "common/memory.h"
#include "posix_net_client_set.h"

enum posix_net_mode {
  net_mode_running,
  net_mode_disconnecting,
  net_mode_stopped
};

struct posix_net_context {
  int HostFD;
  int WakeReadFD;
  int WakeWriteFD;
  int ReadFDMax;
  void *Memory;
  linear_allocator Allocator;
  chunk_ring_buffer CommandRing;
  chunk_ring_buffer EventRing;
  void *EventBufferAddr;
  posix_net_client_set ClientSet;
  posix_net_mode Mode;
  buffer ReceiveBuffer;
  buffer EventOutBuffer;
  buffer CommandSerializationBuffer;
  buffer CommandReadBuffer;
  buffer IncomingReadBuffer;
};

void InitPosixNet(posix_net_context *Context);
void* RunPosixNet(void *Data);
void ShutdownPosixNet(posix_net_context *Context);
void TerminatePosixNet(posix_net_context *Context);
void PosixNetBroadcast(posix_net_context *Context, net_client_id *IDs, memsize Count, buffer Message);
void PosixNetSend(posix_net_context *Context, net_client_id ID, buffer Message);
memsize ReadPosixNetEvent(posix_net_context *Context, buffer Output);
