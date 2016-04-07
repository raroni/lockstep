#ifndef CLIENT_NETWORK_EVENTS_H
#define CLIENT_NETWORK_EVENTS_H

#include "lib/def.h"

#define NETWORK_EVENT_MAX_LENGTH 512

enum network_event_type {
  network_event_type_connection_established,
  network_event_type_connection_failed,
  network_event_type_connection_lost
};

memsize SerializeConnectionEstablishedNetworkEvent(buffer Out);
memsize SerializeConnectionLostNetworkEvent(buffer Out);
memsize SerializeConnectionFailedNetworkEvent(buffer Out);
network_event_type UnserializeNetworkEventType(buffer Input);

#endif
