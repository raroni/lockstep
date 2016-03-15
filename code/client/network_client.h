enum network_state {
  network_state_inactive,
  network_state_connecting,
  network_state_connected,
};

struct network {
  network_state State;
};

extern network Network;

void InitNetwork();
void UpdateNetwork();
void TerminateNetwork();
