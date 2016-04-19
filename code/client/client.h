#pragma once

// TODO: Remove this include
#include "posix_network.h"

struct client_state {
  posix_network_context *TEMP_NETWORK_CONTEXT;
  bool Running;
  bool DisconnectRequested;
};

void InitClient(client_state *State);
void UpdateClient(client_state *State);
