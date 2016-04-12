#pragma once

#include "lib/def.h"

#define MAX_MESSAGE_LENGTH 1024

extern const memsize StartNetworkMesageSize;

enum network_message_type {
  network_message_type_start = 123 // Temp dummy value
};

bool UnserializeNetworkMessageType(buffer Input, network_message_type *Type);
memsize SerializeStartNetworkMessage(buffer Buffer);
memsize GetStartNetworkMesageSize();