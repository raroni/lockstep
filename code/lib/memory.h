#pragma once

#include "lib/def.h"

struct linear_allocator {
  memsize Capacity;
  memsize Length;
  memsize CheckpointCount;
  void *Base;
};

struct linear_allocator_checkpoint {
  linear_allocator *Allocator;
  memsize Length;
};

void InitLinearAllocator(linear_allocator *A, void *Base, memsize Capacity);
void* LinearAllocate(linear_allocator *A, memsize Size);
void TerminateLinearAllocator(linear_allocator *A);
void* GetLinearAllocatorHead(linear_allocator *A);
memsize GetLinearAllocatorFree(linear_allocator *A);

linear_allocator_checkpoint CreateLinearAllocatorCheckpoint(linear_allocator *Allocator);
void ReleaseLinearAllocatorCheckpoint(linear_allocator_checkpoint Checkpoint);
