#include "lib/buf_view.h"
#include "lib/memory.h"
#include "simulation.h"

buffer SerializeOrder(simulation_order Order, linear_allocator *Allocator);
simulation_order UnserializeOrder(buffer Order, linear_allocator *Allocator);
buffer SerializeOrderList(simulation_order_list *List, linear_allocator *Allocator);
simulation_order_list UnserializeOrderList(buffer ListBuffer, linear_allocator *Allocator);
