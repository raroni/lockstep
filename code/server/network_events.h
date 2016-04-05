#ifndef NETWORK_EVENTS_H
#define NETWORK_EVENTS_H

#include "network.h"

#define NETWORK_EVENT_MAX_LENGTH 512

enum network_event_type {
  network_event_type_connect,
  network_event_type_disconnect
};

struct connect_network_event {
  client_id ClientID;
};

struct disconnect_network_event {
  client_id ClientID;
};

network_event_type UnserializeNetworkEventType(void *Buffer, memsize Length);
memsize SerializeDisconnectNetworkEvent(client_id ID, void *Buffer, memsize Length);
memsize SerializeConnectNetworkEvent(client_id ID, void *Buffer, memsize Length);
connect_network_event UnserializeConnectNetworkEvent(void *Buffer, memsize Length);
disconnect_network_event UnserializeDisconnectNetworkEvent(void *Buffer, memsize Length);

#endif
