#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

enum network_event_type {
  network_event_type_connect,
  network_event_type_disconnect
};

struct network_base_event {
  network_event_type Type;
};

void InitNetwork();
void* RunNetwork(void *Data);
void DisconnectNetwork();
void TerminateNetwork();
memsize ReadNetworkEvent(network_base_event **Event);

#endif
