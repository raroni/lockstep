#include "common/memory.h"
#include "simulation.h"

buffer SerializeOrder(simulation_order Order, linear_allocator *Allocator);
simulation_order UnserializeOrder(buffer Order, linear_allocator *Allocator);
