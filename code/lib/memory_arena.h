#pragma once

#include "lib/def.h"

struct memory_arena {
  memsize Capacity;
  memsize Length;
  memsize CheckpointCount;
  void *Base;
};

struct memory_arena_checkpoint {
  memory_arena *Arena;
  memsize Length;
};

void InitMemoryArena(memory_arena *A, void *Base, memsize Capacity);
void* MemoryArenaAllocate(memory_arena *A, memsize Size);
void TerminateMemoryArena(memory_arena *A);
void* GetMemoryArenaHead(memory_arena *A);
memsize GetMemoryArenaFree(memory_arena *A);

memory_arena_checkpoint CreateMemoryArenaCheckpoint(memory_arena *Arena);
void ReleaseMemoryArenaCheckpoint(memory_arena_checkpoint Checkpoint);
