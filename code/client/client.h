#pragma once

#include "lib/chunk_list.h"

struct client_state {
  buffer CommandSerializationBuffer;
  bool Running;
  bool DisconnectRequested;
};

void InitClient(client_state *State);
void UpdateClient(chunk_list *NetEvents, chunk_list *NetCmds, client_state *State);
void TerminateClient(client_state *State);
