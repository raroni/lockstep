#include <string.h>
#include <stdatomic.h>
#include "math.h"
#include "assert.h"
#include "memory_barrier.h"
#include "byte_ring_buffer.h"

typedef byte_ring_buffer brb;

memsize ByteRingBufferCalcUsage(brb *Buffer) {
  memsize Result;
  memsize WritePos = atomic_load_explicit(&Buffer->WritePos, memory_order_acquire);
  memsize ReadPos = atomic_load_explicit(&Buffer->ReadPos, memory_order_acquire);
  if(ReadPos <= WritePos) {
    Result = WritePos - ReadPos;
  }
  else {
    Result = Buffer->Storage.Length - ReadPos + WritePos;
  }
  return Result;
}

memsize ByteRingBufferCalcFree(brb *Buffer) {
  memsize Usage = ByteRingBufferCalcUsage(Buffer);
  return Buffer->Storage.Length - 1 - Usage;
}

void InitByteRingBuffer(brb *Buffer, buffer Storage) {
  Buffer->Storage = Storage;
  Buffer->WritePos = ATOMIC_VAR_INIT(0);
  Buffer->ReadPos = ATOMIC_VAR_INIT(0);
}

void ByteRingBufferWrite(brb *Buffer, buffer Input) {
  Assert(ByteRingBufferCalcFree(Buffer) > Input.Length);

  memsize WritePos = atomic_load_explicit(&Buffer->WritePos, memory_order_relaxed);

  if(Buffer->Storage.Length - WritePos >= Input.Length) {
    void *Destination = ((ui8*)Buffer->Storage.Addr) + WritePos;
    memcpy(Destination, Input.Addr, Input.Length);
  }
  else {
    void *Destination1 = ((ui8*)Buffer->Storage.Addr) + WritePos;
    const void *Source1 = Input.Addr;
    memsize WriteSize1 = Buffer->Storage.Length - WritePos;

    void *Destination2 = Buffer->Storage.Addr;
    const void *Source2 = ((ui8*)Input.Addr) + WriteSize1;
    memsize WriteSize2 = Input.Length - WriteSize1;

    memcpy(Destination1, Source1, WriteSize1);
    memcpy(Destination2, Source2, WriteSize2);
  }

  memsize NewWritePos = (WritePos + Input.Length) % Buffer->Storage.Length;;
  atomic_store_explicit(&Buffer->WritePos, NewWritePos, memory_order_release);
}

memsize ByteRingBufferPeek(brb *ByteRingBuffer, buffer Output) {
  memsize Usage = ByteRingBufferCalcUsage(ByteRingBuffer);
  memsize ReadLength = MinMemsize(Output.Length, Usage);

  memsize ReadPos = atomic_load_explicit(&ByteRingBuffer->ReadPos, memory_order_relaxed);

  if(ByteRingBuffer->Storage.Length - ReadPos >= ReadLength) {
    void *Source = ((ui8*)ByteRingBuffer->Storage.Addr) + ReadPos;
    memcpy(Output.Addr, Source, ReadLength);
  }
  else {
    void *Destination1 = Output.Addr;
    const void *Source1 = ((ui8*)ByteRingBuffer->Storage.Addr + ReadPos);
    memsize ReadLength1 = ByteRingBuffer->Storage.Length - ReadPos;

    void *Destination2 = ((ui8*)Output.Addr) + ReadLength1;
    const void *Source2 = ByteRingBuffer->Storage.Addr;
    memsize ReadLength2 = ReadLength - ReadLength1;

    memcpy(Destination1, Source1, ReadLength1);
    memcpy(Destination2, Source2, ReadLength2);
  }

  return ReadLength;
}

void ByteRingBufferReadAdvance(brb *Ring, memsize Length) {
  memsize OldReadPos = atomic_load_explicit(&Ring->ReadPos, memory_order_relaxed);
  memsize NewReadPos = (OldReadPos + Length) % Ring->Storage.Length;
  atomic_store_explicit(&Ring->ReadPos, NewReadPos, memory_order_release);
}

memsize ByteRingBufferRead(brb *Ring, buffer Output) {
  memsize ReadLength = ByteRingBufferPeek(Ring, Output);
  MemoryBarrier;
  ByteRingBufferReadAdvance(Ring, ReadLength);
  return ReadLength;
}

void TerminateByteRingBuffer(brb *Buffer) {
  ByteRingBufferReset(Buffer);
  Buffer->Storage.Addr = NULL;
  Buffer->Storage.Length = 0;
}

/* Not thread-safe */
void ByteRingBufferReset(brb *Ring) {
  atomic_store_explicit(&Ring->WritePos, 0, memory_order_relaxed);
  atomic_store_explicit(&Ring->ReadPos, 0, memory_order_relaxed);
}
