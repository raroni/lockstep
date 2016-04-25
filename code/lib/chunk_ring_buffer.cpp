#include <string.h>
#include "assert.h"
#include "memory_barrier.h"
#include "chunk_ring_buffer.h"

typedef chunk_ring_buffer crb;

void InitChunkRingBuffer(
  crb *Buffer,
  memsize ChunkCount,
  buffer Storage
) {
  Assert(Storage.Length > sizeof(memsize)*ChunkCount*2);

  Buffer->Offsets = (memsize*)Storage.Addr;

  void *Sizes = ((ui8*)Storage.Addr) + sizeof(memsize)*ChunkCount;
  Buffer->Sizes = (memsize*)Sizes;

  memset(Storage.Addr, 0, sizeof(memsize)*ChunkCount*2);

  buffer Data = {
    .Addr = ((ui8*)Storage.Addr) + sizeof(memsize)*ChunkCount*2,
    .Length = Storage.Length - sizeof(memsize)*ChunkCount*2
  };
  Buffer->Data = Data;

  Buffer->ChunkCount = ChunkCount;
  Buffer->ReadIndex = 0;
  Buffer->WriteIndex = 0;
}

memsize GetChunkRingBufferUnreadCount(chunk_ring_buffer *Buffer) {
  return Buffer->WriteIndex - Buffer->ReadIndex;
}

void ChunkRingBufferWrite(chunk_ring_buffer *Buffer, const buffer Input) {
  memsize NewWriteIndex = (Buffer->WriteIndex + 1) % Buffer->ChunkCount;
  Assert(NewWriteIndex != Buffer->ReadIndex);

  memsize ReadOffset = Buffer->Offsets[Buffer->ReadIndex];
  memsize WriteOffset = Buffer->Offsets[Buffer->WriteIndex];
  if(ReadOffset <= WriteOffset) {
    memsize Capacity = Buffer->Data.Length - WriteOffset;
    if(Input.Length > Capacity) {
      Assert(Input.Length <= ReadOffset);
      Buffer->Offsets[Buffer->WriteIndex] = WriteOffset = 0;
    }
  }
  else {
    memsize Capacity = ReadOffset - WriteOffset;
    Assert(Input.Length <= Capacity);
  }

  Buffer->Sizes[Buffer->WriteIndex] = Input.Length;
  void *Destination = ((ui8*)Buffer->Data.Addr) + WriteOffset;
  memcpy(Destination, Input.Addr, Input.Length);
  Buffer->Offsets[NewWriteIndex] = WriteOffset + Input.Length;

  MemoryBarrier;

  Buffer->WriteIndex = NewWriteIndex;
}

buffer ChunkRingBufferPeek(chunk_ring_buffer *Buffer) {
  buffer Result;
  if(Buffer->ReadIndex == Buffer->WriteIndex) {
    Result.Addr = NULL;
    Result.Length = 0;
    return Result;
  }
  memsize ReadOffset = Buffer->Offsets[Buffer->ReadIndex];
  Result.Addr = ((ui8*)Buffer->Data.Addr) + ReadOffset;
  Result.Length = Buffer->Sizes[Buffer->ReadIndex];
  return Result;
}

buffer ChunkRingBufferRefRead(chunk_ring_buffer *Buffer) {
  buffer Peek = ChunkRingBufferPeek(Buffer);
  if(Peek.Length != 0) {
    ChunkRingBufferReadAdvance(Buffer);
  }
  return Peek;
}

void ChunkRingBufferReadAdvance(chunk_ring_buffer *Buffer) {
  Buffer->ReadIndex = (Buffer->ReadIndex + 1) % Buffer->ChunkCount;
}

memsize ChunkRingBufferCopyRead(chunk_ring_buffer *Buffer, buffer Output) {
  buffer Peek = ChunkRingBufferPeek(Buffer);
  if(Peek.Length == 0) {
    return 0;
  }

  Assert(Output.Length >= Peek.Length);
  memcpy(Output.Addr, Peek.Addr, Peek.Length);

  MemoryBarrier;

  ChunkRingBufferReadAdvance(Buffer);

  return Peek.Length;
}

void TerminateChunkRingBuffer(crb *Buffer) {
  Buffer->WriteIndex = 0;
  Buffer->ChunkCount = 0;
  Buffer->Offsets = NULL;
  Buffer->Sizes = NULL;
  Buffer->Data.Addr = NULL;
  Buffer->Data.Length = 0;
  Buffer->ReadIndex = 0;
}
