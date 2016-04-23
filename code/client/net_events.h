#pragma once

#include "lib/def.h"

#define NETWORK_EVENT_MAX_LENGTH 512

enum net_event_type {
  net_event_type_connection_established,
  net_event_type_connection_failed,
  net_event_type_connection_lost,
  net_event_type_message
};

struct message_net_event {
  buffer Message;
};

memsize SerializeConnectionEstablishedNetEvent(buffer Out);
memsize SerializeConnectionLostNetEvent(buffer Out);
memsize SerializeConnectionFailedNetEvent(buffer Out);
memsize SerializeMessageNetEvent(buffer Message, buffer Out);
message_net_event UnserializeMessageNetEvent(buffer Event);
net_event_type UnserializeNetEventType(buffer Input);
