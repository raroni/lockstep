#ifndef SERVER_NETWORK_COMMANDS_H
#define SERVER_NETWORK_COMMANDS_H

#include "network.h"

#define COMMAND_MAX_LENGTH 512

enum network_command_type {
  network_command_type_disconnect,
  network_command_type_broadcast
};

struct broadcast_network_command {
  client_id *ClientIDs;
  memsize ClientIDCount;
  void *Message;
  memsize MessageLength;
};

memsize SerializeDisconnectNetworkCommand(void *Buffer, memsize Capacity);
memsize SerializeBroadcastNetworkCommand(
    const client_id *IDs,
    memsize IDCount,
    const void *Message,
    memsize MessageLength,
    void *Buffer,
    memsize BufferCapacity
);
network_command_type UnserializeNetworkCommandType(void *Buffer, memsize Capacity);
broadcast_network_command UnserializeBroadcastNetworkCommand(void *Buffer, memsize Capacity);

#endif
