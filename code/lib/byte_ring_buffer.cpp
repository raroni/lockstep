#include <string.h>
#include "assert.h"
#include "byte_ring_buffer.h"

typedef byte_ring_buffer brb;

memsize ByteRingBufferCalcUsage(brb *Buffer) {
  memsize Result;
  if(Buffer->ReadPos <= Buffer->WritePos) {
    Result = Buffer->WritePos - Buffer->ReadPos;
  }
  else {
    Result = Buffer->Capacity - Buffer->ReadPos + Buffer->WritePos;
  }
  return Result;
}

memsize ByteRingBufferCalcFree(brb *Buffer) {
  memsize Usage = ByteRingBufferCalcUsage(Buffer);
  return Buffer->Capacity - 1 - Usage;
}

void InitByteRingBuffer(brb *Buffer, void *Data, memsize Capacity) {
  Buffer->Data = Data;
  Buffer->WritePos = 0;
  Buffer->ReadPos = 0;
  Buffer->Capacity = Capacity;
}

void ByteRingBufferWrite(brb *Buffer, const void *Source, memsize Length) {
  Assert(ByteRingBufferCalcFree(Buffer) > Length);

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

memsize ByteRingBufferRead(brb *ByteRingBuffer, void *ReadBuffer, memsize MaxLength) {
  memsize Usage = ByteRingBufferCalcUsage(ByteRingBuffer);
  memsize ReadLength = MinMemsize(MaxLength, Usage);

  if(ByteRingBuffer->Capacity - ByteRingBuffer->ReadPos >= ReadLength) {
    void *Source = ((ui8*)ByteRingBuffer->Data) + ByteRingBuffer->ReadPos;
    memcpy(ReadBuffer, Source, ReadLength);
  }
  else {
    void *Destination1 = ReadBuffer;
    const void *Source1 = ((ui8*)ByteRingBuffer->Data + ByteRingBuffer->ReadPos);
    memsize ReadLength1 = ByteRingBuffer->Capacity - ByteRingBuffer->ReadPos;

    void *Destination2 = ((ui8*)ReadBuffer) + ReadLength1;
    const void *Source2 = ByteRingBuffer->Data;
    memsize ReadLength2 = ReadLength - ReadLength1;

    memcpy(Destination1, Source1, ReadLength1);
    memcpy(Destination2, Source2, ReadLength2);
  }

  MemoryBarrier;

  ByteRingBuffer->ReadPos = (ByteRingBuffer->ReadPos + ReadLength) % ByteRingBuffer->Capacity;
  return ReadLength;
}

void TerminateByteRingBuffer(brb *Buffer) {
  Buffer->Data = NULL;
  Buffer->WritePos = 0;
  Buffer->ReadPos = 0;
  Buffer->Capacity = 0;
}
