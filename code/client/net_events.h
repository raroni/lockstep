#pragma once

#include "lib/def.h"
#include "lib/memory_arena.h"

#define NET_EVENT_MAX_LENGTH 512

enum net_event_type {
  net_event_type_connection_established,
  net_event_type_connection_failed,
  net_event_type_connection_lost,
  net_event_type_message
};

struct message_net_event {
  buffer Message;
};

buffer SerializeConnectionEstablishedNetEvent(memory_arena *Arena);
buffer SerializeConnectionLostNetEvent(memory_arena *Arena);
buffer SerializeConnectionFailedNetEvent(memory_arena *Arena);
buffer SerializeMessageNetEvent(buffer Message, memory_arena *Arena);
message_net_event UnserializeMessageNetEvent(buffer Event);
net_event_type UnserializeNetEventType(buffer Input);
