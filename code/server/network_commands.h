#pragma once

#include "network.h"

#define COMMAND_MAX_LENGTH 512

enum network_command_type {
  network_command_type_disconnect,
  network_command_type_broadcast
};

struct broadcast_network_command {
  client_id *ClientIDs;
  memsize ClientIDCount;
  buffer Message;
};

memsize SerializeDisconnectNetworkCommand(buffer Buffer);
memsize SerializeBroadcastNetworkCommand(const client_id *IDs, memsize IDCount, const buffer Message, buffer Out);
network_command_type UnserializeNetworkCommandType(buffer Buffer);
broadcast_network_command UnserializeBroadcastNetworkCommand(buffer Buffer);
