#pragma once

#define COMMAND_MAX_LENGTH 512

enum network_command_type {
  network_command_type_shutdown,
  network_command_type_send
};

struct send_network_command {
  buffer Message;
};


memsize SerializeShutdownNetworkCommand(buffer Buffer);
memsize SerializeSendNetworkCommand(buffer Output, buffer Message);
network_command_type UnserializeNetworkCommandType(buffer Buffer);
send_network_command UnserializeSendNetworkCommand(buffer Buffer);
