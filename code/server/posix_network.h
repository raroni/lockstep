#pragma once

#include "lib/def.h"
#include "lib/chunk_ring_buffer.h"
#include "client_set.h"

enum network_mode {
  network_mode_running,
  network_mode_disconnecting,
  network_mode_stopped
};

struct network_context {
  int HostFD;
  int WakeReadFD;
  int WakeWriteFD;
  int ReadFDMax;
  chunk_ring_buffer CommandRing;
  void *CommandBufferAddr;
  chunk_ring_buffer EventRing;
  void *EventBufferAddr;
  client_set ClientSet;
  network_mode Mode;
  buffer ReceiveBuffer;
  buffer EventOutBuffer;
  buffer CommandSerializationBuffer;
  buffer CommandReadBuffer;
  buffer IncomingReadBuffer;
};

void InitNetwork(network_context *Context);
void* RunNetwork(void *Data);
void ShutdownNetwork(network_context *Context);
void TerminateNetwork(network_context *Context);
void NetworkBroadcast(network_context *Context, client_id *IDs, memsize Count, buffer Message);
memsize ReadNetworkEvent(network_context *Context, buffer Output);
