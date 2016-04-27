#include "lib/serialization.h"
#include "common/memory.h"
#include "simulation.h"

memsize SerializeOrder(simulation_order Order, buffer Output);
simulation_order UnserializeOrder(buffer Order, linear_allocator *Allocator);
memsize SerializeOrderList(simulation_order_list *List, buffer Output);
simulation_order_list UnserializeOrderList(buffer ListBuffer, linear_allocator *Allocator);
