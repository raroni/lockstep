#pragma once

#include "lib/def.h"

#define MAX_MESSAGE_LENGTH 1024

extern const memsize StartNetMessageSize;
extern const memsize ReplyNetMessageSize;

enum net_message_type {
  net_message_type_start = 123, // Temp dummy value
  net_message_type_reply
};

struct start_net_message {
  memsize PlayerCount;
  memsize PlayerID;
};

memsize SerializeStartNetMessage(memsize PlayerCount, memsize PlayerID, buffer Buffer);
memsize SerializeReplyNetMessage(buffer Buffer);
bool UnserializeNetMessageType(buffer Input, net_message_type *Type);
bool UnserializeStartNetMessage(buffer Buffer, start_net_message *Message);
