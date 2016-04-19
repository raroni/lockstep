#pragma once

#include "lib/chunk_list.h"

// TODO: Remove this include
#include "posix_network.h"

struct client_state {
  posix_network_context *TEMP_NETWORK_CONTEXT;
  buffer CommandSerializationBuffer;
  bool Running;
  bool DisconnectRequested;
};

void InitClient(client_state *State);
void UpdateClient(chunk_list *NetCmds, client_state *State);
void TerminateClient(client_state *State);
