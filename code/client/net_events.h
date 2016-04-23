#pragma once

#include "lib/def.h"

#define NETWORK_EVENT_MAX_LENGTH 512

enum net_event_type {
  net_event_type_connection_established,
  net_event_type_connection_failed,
  net_event_type_connection_lost,
  net_event_type_start
};

struct start_net_event {
  memsize PlayerCount;
  memsize PlayerID;
};

memsize SerializeConnectionEstablishedNetEvent(buffer Out);
memsize SerializeConnectionLostNetEvent(buffer Out);
memsize SerializeConnectionFailedNetEvent(buffer Out);
memsize SerializeStartNetEvent(buffer Out, memsize PlayerCount, memsize PlayerID);
start_net_event UnserializeStartNetEvent(buffer Event);
net_event_type UnserializeNetEventType(buffer Input);
