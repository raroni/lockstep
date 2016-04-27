#pragma once

#include "lib/def.h"
#include "lib/math.h"
#include "common/memory.h"
#include "common/simulation.h"

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

struct order_list_net_message {
  memsize Count;
};

struct order_net_message {
  simulation_unit_id *UnitIDs;
  memsize UnitCount;
  ivec2 Target;
};

memsize SerializeStartNetMessage(memsize PlayerCount, memsize PlayerIndex, buffer Buffer);
memsize SerializeOrderNetMessage(simulation_unit_id *UnitIDs, memsize UnitCount, ivec2 Target, buffer Out);
memsize SerializeReplyNetMessage(buffer Buffer);
memsize SerializeOrderListNetMessage(buffer Out);
net_message_type UnserializeNetMessageType(buffer Input);
order_net_message UnserializeOrderNetMessage(buffer Input, linear_allocator *Allocator);
start_net_message UnserializeStartNetMessage(buffer Input);
order_list_net_message UnserializeOrderListNetMessage(buffer Input);
bool ValidateStartNetMessage(start_net_message Message);
bool ValidateOrderListNetMessage(order_list_net_message Message);
bool ValidateOrderNetMessage(order_net_message Message);
bool ValidateNetMessageType(net_message_type Type);
