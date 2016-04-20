#pragma once

#include "lib/chunk_list.h"

struct client_memory {
  buffer MemoryPool;
  bool Running;
  bool DisconnectRequested; // <- move this to keyboard input
};

void InitClient(client_memory *State);
void UpdateClient(chunk_list *NetEvents, chunk_list *NetCmds, client_memory *Memory);
