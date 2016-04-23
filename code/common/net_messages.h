#pragma once

#include "lib/def.h"

#define MAX_MESSAGE_LENGTH 1024

extern const memsize MinMessageSize;
extern const memsize StartNetMessageSize;
extern const memsize ReplyNetMessageSize;

enum net_message_type {
  net_message_type_start = 123, // Temp dummy value
  net_message_type_reply,
  net_message_type_count
};

struct start_net_message {
  memsize PlayerCount;
  memsize PlayerID;
};

memsize SerializeStartNetMessage(memsize PlayerCount, memsize PlayerID, buffer Buffer);
memsize SerializeReplyNetMessage(buffer Buffer);
net_message_type UnserializeNetMessageType(buffer Input);
start_net_message UnserializeStartNetMessage(buffer Input);
bool ValidateMessageLength(buffer Buffer, net_message_type Type);
bool ValidateStartNetMessage(start_net_message Message);
bool ValidateNetMessageType(net_message_type Type);
