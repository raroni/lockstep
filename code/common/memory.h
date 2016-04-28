#pragma once

#include "lib/def.h"

struct linear_allocator {
  memsize Capacity;
  memsize Length;
  void *Base;
};

struct linear_allocator_context {
  linear_allocator *Allocator;
  memsize Length;
};

void InitLinearAllocator(linear_allocator *A, void *Base, memsize Capacity);
void* LinearAllocate(linear_allocator *A, memsize Size);
void TerminateLinearAllocator(linear_allocator *A);
void* GetLinearAllocatorHead(linear_allocator *A);

linear_allocator_context CreateLinearAllocatorContext(linear_allocator *Allocator);
void RestoreLinearAllocatorContext(linear_allocator_context Context);
