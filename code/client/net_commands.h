#pragma once

#include "lib/memory_arena.h"

#define NET_COMMAND_MAX_LENGTH 512

enum net_command_type {
  net_command_type_shutdown,
  net_command_type_send
};

struct send_net_command {
  buffer Message;
};

buffer SerializeShutdownNetCommand(memory_arena *Arena);
buffer SerializeSendNetCommand(buffer Message, memory_arena *Arena);
net_command_type UnserializeNetCommandType(buffer Buffer);
send_net_command UnserializeSendNetCommand(buffer Buffer);
