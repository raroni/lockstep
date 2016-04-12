#include <string.h>
#include "min_max.h"
#include "assert.h"
#include "memory_barrier.h"
#include "byte_ring_buffer.h"

typedef byte_ring_buffer brb;

memsize ByteRingBufferCalcUsage(brb *Buffer) {
  memsize Result;
  if(Buffer->ReadPos <= Buffer->WritePos) {
    Result = Buffer->WritePos - Buffer->ReadPos;
  }
  else {
    Result = Buffer->Storage.Length - Buffer->ReadPos + Buffer->WritePos;
  }
  return Result;
}

memsize ByteRingBufferCalcFree(brb *Buffer) {
  memsize Usage = ByteRingBufferCalcUsage(Buffer);
  return Buffer->Storage.Length - 1 - Usage;
}

void InitByteRingBuffer(brb *Buffer, buffer Storage) {
  Buffer->Storage = Storage;
  Buffer->WritePos = 0;
  Buffer->ReadPos = 0;
}

void ByteRingBufferWrite(brb *Buffer, buffer Input) {
  Assert(ByteRingBufferCalcFree(Buffer) > Input.Length);

  if(Buffer->Storage.Length - Buffer->WritePos >= Input.Length) {
    void *Destination = ((ui8*)Buffer->Storage.Addr) + Buffer->WritePos;
    memcpy(Destination, Input.Addr, Input.Length);
  }
  else {
    void *Destination1 = ((ui8*)Buffer->Storage.Addr) + Buffer->WritePos;
    const void *Source1 = Input.Addr;
    memsize WriteSize1 = Buffer->Storage.Length - Buffer->WritePos;

    void *Destination2 = Buffer->Storage.Addr;
    const void *Source2 = ((ui8*)Input.Addr) + WriteSize1;
    memsize WriteSize2 = Input.Length - WriteSize1;

    memcpy(Destination1, Source1, WriteSize1);
    memcpy(Destination2, Source2, WriteSize2);
  }

  MemoryBarrier;

  Buffer->WritePos = (Buffer->WritePos + Input.Length) % Buffer->Storage.Length;
}

memsize ByteRingBufferPeek(brb *ByteRingBuffer, buffer Output) {
  memsize Usage = ByteRingBufferCalcUsage(ByteRingBuffer);
  memsize ReadLength = MinMemsize(Output.Length, Usage);

  if(ByteRingBuffer->Storage.Length - ByteRingBuffer->ReadPos >= ReadLength) {
    void *Source = ((ui8*)ByteRingBuffer->Storage.Addr) + ByteRingBuffer->ReadPos;
    memcpy(Output.Addr, Source, ReadLength);
  }
  else {
    void *Destination1 = Output.Addr;
    const void *Source1 = ((ui8*)ByteRingBuffer->Storage.Addr + ByteRingBuffer->ReadPos);
    memsize ReadLength1 = ByteRingBuffer->Storage.Length - ByteRingBuffer->ReadPos;

    void *Destination2 = ((ui8*)Output.Addr) + ReadLength1;
    const void *Source2 = ByteRingBuffer->Storage.Addr;
    memsize ReadLength2 = ReadLength - ReadLength1;

    memcpy(Destination1, Source1, ReadLength1);
    memcpy(Destination2, Source2, ReadLength2);
  }

  return ReadLength;
}

void ByteRingBufferReadAdvance(brb *Ring, memsize Length) {
  Ring->ReadPos = (Ring->ReadPos + Length) % Ring->Storage.Length;
}

memsize ByteRingBufferRead(brb *Ring, buffer Output) {
  memsize ReadLength = ByteRingBufferPeek(Ring, Output);
  MemoryBarrier;
  ByteRingBufferReadAdvance(Ring, ReadLength);
  return ReadLength;
}

void ByteRingBufferReset(brb *Ring) {
  Ring->WritePos = 0;
  Ring->ReadPos = 0;
}

void TerminateByteRingBuffer(brb *Buffer) {
  Buffer->Storage.Addr = NULL;
  Buffer->WritePos = 0;
  Buffer->ReadPos = 0;
  Buffer->Storage.Length = 0;
}
