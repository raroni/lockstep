#pragma once

#include "lib/def.h"

#define MAX_MESSAGE_LENGTH 1024

extern const memsize MinMessageSize;
extern const memsize StartNetMessageSize;
extern const memsize ReplyNetMessageSize;
extern const memsize OrderSetNetMessageSize;

enum net_message_type {
  net_message_type_start = 123, // Temp dummy value
  net_message_type_order_set,
  net_message_type_reply,
  net_message_type_count
};

struct start_net_message {
  memsize PlayerCount;
  memsize PlayerIndex;
};

struct order_set_net_message {
  memsize Count;
};

memsize SerializeStartNetMessage(memsize PlayerCount, memsize PlayerIndex, buffer Buffer);
memsize SerializeReplyNetMessage(buffer Buffer);
memsize SerializeOrderSetNetMessage(buffer Out);
net_message_type UnserializeNetMessageType(buffer Input);
start_net_message UnserializeStartNetMessage(buffer Input);
order_set_net_message UnserializeOrderSetNetMessage(buffer Input);
bool ValidateMessageLength(buffer Buffer, net_message_type Type);
bool ValidateStartNetMessage(start_net_message Message);
bool ValidateOrderSetNetMessage(order_set_net_message Message);
bool ValidateNetMessageType(net_message_type Type);
