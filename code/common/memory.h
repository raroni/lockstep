#ifndef MEMORY_H
#define MEMORY_H

#include "shared.h"

struct linear_allocator {
  memsize Capacity;
  memsize Length;
  void *Base;
};

void InitLinearAllocator(linear_allocator *A, void *Base, memsize Capacity);
void* LinearAllocate(linear_allocator *A, memsize Size);
void TerminateLinearAllocator(linear_allocator *A);

#endif
