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

memsize ChunkRingBufferRead(chunk_ring_buffer *Buffer, buffer Output) {
  if(Buffer->ReadIndex == Buffer->WriteIndex) {
    return 0;
  }
  memsize ChunkSize = Buffer->Sizes[Buffer->ReadIndex];

  Assert(Output.Length >= ChunkSize);

  memsize ReadOffset = Buffer->Offsets[Buffer->ReadIndex];
  void *Source = ((ui8*)Buffer->Data.Addr) + ReadOffset;
  memcpy(Output.Addr, Source, ChunkSize);

  MemoryBarrier;

  Buffer->ReadIndex = (Buffer->ReadIndex + 1) % Buffer->ChunkCount;
  return ChunkSize;
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
