#pragma once

#include "lib/def.h"

#define NETWORK_EVENT_MAX_LENGTH 512

enum network_event_type {
  network_event_type_connection_established,
  network_event_type_connection_failed,
  network_event_type_connection_lost,
  network_event_type_start
};

memsize SerializeConnectionEstablishedNetworkEvent(buffer Out);
memsize SerializeConnectionLostNetworkEvent(buffer Out);
memsize SerializeConnectionFailedNetworkEvent(buffer Out);
memsize SerializeStartNetworkEvent(buffer Out);
network_event_type UnserializeNetworkEventType(buffer Input);
