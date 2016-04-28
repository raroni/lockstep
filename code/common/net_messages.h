#pragma once

#include "lib/def.h"
#include "lib/math.h"
#include "lib/memory_arena.h"

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

buffer SerializeStartNetMessage(memsize PlayerCount, memsize PlayerIndex, memory_arena *Arena);
buffer SerializeOrderNetMessage(ui16 *UnitIDs, memsize UnitCount, ivec2 Target, memory_arena *Arena);
buffer SerializeReplyNetMessage(memory_arena *Arena);
buffer SerializeOrderListNetMessage(net_message_order *Orders, ui16 OrderCount, memory_arena *Arena);
net_message_type UnserializeNetMessageType(buffer Input);
order_net_message UnserializeOrderNetMessage(buffer Input, memory_arena *Arena);
start_net_message UnserializeStartNetMessage(buffer Input);
order_list_net_message UnserializeOrderListNetMessage(buffer Input, memory_arena *Arena);
bool ValidateStartNetMessage(start_net_message Message);
bool ValidateOrderListNetMessage(order_list_net_message Message);
bool ValidateOrderNetMessage(order_net_message Message);
bool ValidateNetMessageType(net_message_type Type);
