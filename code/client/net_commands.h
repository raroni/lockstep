#pragma once

#define NETWORK_COMMAND_MAX_LENGTH 512

enum net_command_type {
  net_command_type_shutdown,
  net_command_type_send
};

struct send_net_command {
  buffer Message;
};

memsize SerializeShutdownNetCommand(buffer Buffer);
memsize SerializeSendNetCommand(buffer Output, buffer Message);
net_command_type UnserializeNetCommandType(buffer Buffer);
send_net_command UnserializeSendNetCommand(buffer Buffer);
