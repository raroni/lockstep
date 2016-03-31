#include <string.h>
#include "assert.h"
#include "ring_buffer.h"

memsize RingBufferCalcUsage(ring_buffer *Buffer) {
  memsize Result;
  if(Buffer->ReadPos <= Buffer->WritePos) {
    Result = Buffer->WritePos - Buffer->ReadPos;
  }
  else {
    Result = Buffer->Capacity - Buffer->ReadPos + Buffer->WritePos;
  }
  return Result;
}

memsize RingBufferCalcFree(ring_buffer *Buffer) {
  memsize Usage = RingBufferCalcUsage(Buffer);
  return Buffer->Capacity - 1 - Usage;
}

void InitRingBuffer(ring_buffer *Buffer, void *Data, memsize Capacity) {
  Buffer->Data = Data;
  Buffer->WritePos = 0;
  Buffer->ReadPos = 0;
  Buffer->Capacity = Capacity;
}

void RingBufferWrite(ring_buffer *Buffer, const void *Source, memsize Length) {
  Assert(RingBufferCalcFree(Buffer) > Length);

  if(Buffer->Capacity - Buffer->WritePos >= Length) {
    void *Destination = ((ui8*)Buffer->Data) + Buffer->WritePos;
    memcpy(Destination, Source, Length);
  }
  else {
    void *Destination1 = ((ui8*)Buffer->Data) + Buffer->WritePos;
    const void *Source1 = Source;
    memsize WriteSize1 = Buffer->Capacity - Buffer->WritePos;

    void *Destination2 = Buffer->Data;
    const void *Source2 = ((ui8*)Source) + WriteSize1;
    memsize WriteSize2 = Length - WriteSize1;

    memcpy(Destination1, Source1, WriteSize1);
    memcpy(Destination2, Source2, WriteSize2);
  }

  MemoryBarrier;

  Buffer->WritePos = (Buffer->WritePos + Length) % Buffer->Capacity;
}

memsize RingBufferRead(ring_buffer *RingBuffer, void *ReadBuffer, memsize MaxLength) {
  memsize Usage = RingBufferCalcUsage(RingBuffer);
  memsize ReadLength = MinMemsize(MaxLength, Usage);

  if(RingBuffer->Capacity - RingBuffer->ReadPos >= ReadLength) {
    void *Source = ((ui8*)RingBuffer->Data) + RingBuffer->ReadPos;
    memcpy(ReadBuffer, Source, ReadLength);
  }
  else {
    void *Destination1 = ReadBuffer;
    const void *Source1 = ((ui8*)RingBuffer->Data + RingBuffer->ReadPos);
    memsize ReadLength1 = RingBuffer->Capacity - RingBuffer->ReadPos;

    void *Destination2 = ((ui8*)ReadBuffer) + ReadLength1;
    const void *Source2 = RingBuffer->Data;
    memsize ReadLength2 = ReadLength - ReadLength1;

    memcpy(Destination1, Source1, ReadLength1);
    memcpy(Destination2, Source2, ReadLength2);
  }

  MemoryBarrier;

  RingBuffer->ReadPos = (RingBuffer->ReadPos + ReadLength) % RingBuffer->Capacity;
  return ReadLength;
}

void TerminateRingBuffer(ring_buffer *Buffer) {
  Buffer->Data = NULL;
  Buffer->WritePos = 0;
  Buffer->ReadPos = 0;
  Buffer->Capacity = 0;
}
