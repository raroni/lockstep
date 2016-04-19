#pragma once

struct client_state {
  bool Running;
  bool DisconnectRequested;
};

void InitClient(client_state *State);
void UpdateClient(client_state *State);
