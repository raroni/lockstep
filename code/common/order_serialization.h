#include "lib/buf_view.h"
#include "lib/memory_arena.h"
#include "simulation.h"

buffer SerializeOrder(simulation_order Order, memory_arena *Arena);
simulation_order UnserializeOrder(buffer Order, memory_arena *Arena);
buffer SerializeOrderList(simulation_order_list *List, memory_arena *Arena);
simulation_order_list UnserializeOrderList(buffer ListBuffer, memory_arena *Arena);
