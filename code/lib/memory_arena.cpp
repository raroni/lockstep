#include "lib/def.h"
#include "lib/assert.h"
#include "memory_arena.h"

void InitMemoryArena(memory_arena *A, void *Base, memsize Capacity) {
  A->Base = Base;
  A->Length = 0;
  A->CheckpointCount = 0;
  A->Capacity = Capacity;
}

void* MemoryArenaAllocate(memory_arena *A, memsize Size) {
  Assert(Size != 0);
  Assert(A->Capacity >= A->Length+Size);
  void *Result = GetMemoryArenaHead(A);
  A->Length += Size;
  return Result;
}

memsize GetMemoryArenaFree(memory_arena *A) {
  return A->Capacity - A->Length;
}

void* GetMemoryArenaHead(memory_arena *A) {
  return (ui8*)A->Base + A->Length;
}

void TerminateMemoryArena(memory_arena *A) {
  A->Base = NULL;
  A->Length = 0;
  A->Capacity = 0;
}

memory_arena_checkpoint CreateMemoryArenaCheckpoint(memory_arena *Arena) {
  memory_arena_checkpoint CP;
  CP.Arena = Arena;
  CP.Length = Arena->Length;
  Arena->CheckpointCount++;
  return CP;
}

void ReleaseMemoryArenaCheckpoint(memory_arena_checkpoint CP) {
  Assert(CP.Length <= CP.Arena->Length);
  CP.Arena->CheckpointCount--;

  if(CP.Arena->CheckpointCount == 0) {
    CP.Arena->Length = CP.Length;
  }
}
