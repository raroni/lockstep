#include "lib/def.h"
#include "lib/assert.h"
#include "memory.h"

void InitLinearAllocator(linear_allocator *A, void *Base, memsize Capacity) {
  A->Base = Base;
  A->Length = 0;
  A->Capacity = Capacity;
}

void* LinearAllocate(linear_allocator *A, memsize Size) {
  Assert(A->Capacity >= A->Length+Size);
  void *Result = (ui8*)A->Base + Size;
  A->Length += Size;
  return Result;
}

void TerminateLinearAllocator(linear_allocator *A) {
  A->Base = NULL;
  A->Length = 0;
  A->Capacity = 0;
}
