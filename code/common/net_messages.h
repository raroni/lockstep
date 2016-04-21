#pragma once

#include "lib/def.h"

#define MAX_MESSAGE_LENGTH 1024

extern const memsize StartNetMesageSize;
extern const memsize ReplyNetMesageSize;

enum net_message_type {
  net_message_type_start = 123, // Temp dummy value
  net_message_type_reply
};

bool UnserializeNetMessageType(buffer Input, net_message_type *Type);
memsize SerializeStartNetMessage(buffer Buffer);
memsize SerializeReplyNetMessage(buffer Buffer);
memsize GetStartNetMesageSize();
