#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

#include "lib/def.h"

#define NETWORK_EVENT_MAX_LENGTH 512

typedef memsize client_id;

enum network_event_type {
  network_event_type_connect,
  network_event_type_disconnect
};

struct network_base_event {
  network_event_type Type;
};

struct network_connect_event {
  network_base_event Base;
  client_id ClientID;
};

struct network_disconnect_event {
  network_base_event Base;
  client_id ClientID;
};

void InitNetwork();
void* RunNetwork(void *Data);
void DisconnectNetwork();
void TerminateNetwork();
void NetworkBroadcast(client_id *IDs, memsize Count, void *Packet, memsize Length);
memsize ReadNetworkEvent(network_base_event *Event, memsize MaxLength);

#endif
