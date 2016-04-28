#pragma once

#include "lib/def.h"
#include "lib/memory.h"

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

buffer SerializeConnectionEstablishedNetEvent(linear_allocator *Allocator);
buffer SerializeConnectionLostNetEvent(linear_allocator *Allocator);
buffer SerializeConnectionFailedNetEvent(linear_allocator *Allocator);
buffer SerializeMessageNetEvent(buffer Message, linear_allocator *Allocator);
message_net_event UnserializeMessageNetEvent(buffer Event);
net_event_type UnserializeNetEventType(buffer Input);
