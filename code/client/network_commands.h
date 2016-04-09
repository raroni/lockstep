#pragma once

#include "network.h"

#define COMMAND_MAX_LENGTH 512

enum network_command_type {
  network_command_type_shutdown
};

memsize SerializeShutdownNetworkCommand(buffer Buffer);
network_command_type UnserializeNetworkCommandType(buffer Buffer);
