#include "lib/def.h"
#include "lib/assert.h"
#include "memory.h"

void InitLinearAllocator(linear_allocator *A, void *Base, memsize Capacity) {
  A->Base = Base;
  A->Length = 0;
  A->Capacity = Capacity;
}

void* LinearAllocate(linear_allocator *A, memsize Size) {
  Assert(Size != 0);
  Assert(A->Capacity >= A->Length+Size);
  void *Result = GetLinearAllocatorHead(A);
  A->Length += Size;
  return Result;
}

memsize GetLinearAllocatorFree(linear_allocator *A) {
  return A->Capacity - A->Length;
}

void* GetLinearAllocatorHead(linear_allocator *A) {
  return (ui8*)A->Base + A->Length;
}

void TerminateLinearAllocator(linear_allocator *A) {
  A->Base = NULL;
  A->Length = 0;
  A->Capacity = 0;
}

linear_allocator_context CreateLinearAllocatorContext(linear_allocator *Allocator) {
  linear_allocator_context C;
  C.Allocator = Allocator;
  C.Length = Allocator->Length;
  return C;
}

void RestoreLinearAllocatorContext(linear_allocator_context Context) {
  Assert(Context.Length <= Context.Allocator->Length);
  Context.Allocator->Length = Context.Length;
}
