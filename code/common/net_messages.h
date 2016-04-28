#pragma once

#include "lib/def.h"
#include "lib/math.h"
#include "common/memory.h"

#define NET_MESSAGE_MAX_LENGTH 1024

enum net_message_type {
  net_message_type_start = 123, // Temp dummy value
  net_message_type_order_list,
  net_message_type_order,
  net_message_type_reply,
  net_message_type_count
};

struct start_net_message {
  memsize PlayerCount;
  memsize PlayerIndex;
};

struct net_message_order {
  ui8 PlayerID;
  ui16 *UnitIDs;
  ui16 UnitCount;
  ivec2 Target;
};

struct order_list_net_message {
  net_message_order *Orders;
  ui16 Count;
};

struct order_net_message {
  ui16 *UnitIDs;
  memsize UnitCount;
  ivec2 Target;
};

buffer SerializeStartNetMessage(memsize PlayerCount, memsize PlayerIndex, linear_allocator *Allocator);
buffer SerializeOrderNetMessage(ui16 *UnitIDs, memsize UnitCount, ivec2 Target, linear_allocator *Allocator);
buffer SerializeReplyNetMessage(linear_allocator *Allocator);
buffer SerializeOrderListNetMessage(net_message_order *Orders, ui16 OrderCount, linear_allocator *Allocator);
net_message_type UnserializeNetMessageType(buffer Input);
order_net_message UnserializeOrderNetMessage(buffer Input, linear_allocator *Allocator);
start_net_message UnserializeStartNetMessage(buffer Input);
order_list_net_message UnserializeOrderListNetMessage(buffer Input, linear_allocator *Allocator);
bool ValidateStartNetMessage(start_net_message Message);
bool ValidateOrderListNetMessage(order_list_net_message Message);
bool ValidateOrderNetMessage(order_net_message Message);
bool ValidateNetMessageType(net_message_type Type);
