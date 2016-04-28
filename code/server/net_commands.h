#pragma once

#include "lib/memory.h"
#include "net.h"

#define NET_COMMAND_MAX_LENGTH 512

enum net_command_type {
  net_command_type_shutdown,
  net_command_type_broadcast,
  net_command_type_send
};

struct broadcast_net_command {
  net_client_id *ClientIDs;
  memsize ClientIDCount;
  buffer Message;
};

struct send_net_command {
  net_client_id ClientID;
  buffer Message;
};

buffer SerializeShutdownNetCommand(linear_allocator *Allocator);
buffer SerializeBroadcastNetCommand(const net_client_id *IDs, memsize IDCount, const buffer Message, linear_allocator *Allocator);
buffer SerializeSendNetCommand(net_client_id ID, const buffer Message, linear_allocator *Allocator);
net_command_type UnserializeNetCommandType(buffer Buffer);
broadcast_net_command UnserializeBroadcastNetCommand(buffer Buffer);
send_net_command UnserializeSendNetCommand(buffer Buffer);
