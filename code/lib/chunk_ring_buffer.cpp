#include <string.h>
#include "chunk_ring_buffer.h"

typedef chunk_ring_buffer crb;

void InitChunkRingBuffer(
  crb *Buffer,
  memsize ChunkCount,
  void *Data,
  memsize DataLength
) {
  Assert(DataLength > sizeof(memsize)*ChunkCount*2);

  Buffer->Offsets = (memsize*)Data;

  void *Sizes = ((ui8*)Data) + sizeof(memsize)*ChunkCount;
  Buffer->Sizes = (memsize*)Sizes;

  memset(Data, 0, sizeof(memsize)*ChunkCount*2);

  Buffer->Data = ((ui8*)Data) + sizeof(memsize)*ChunkCount*2;
  Buffer->DataCapacity = DataLength - sizeof(memsize)*ChunkCount*2;

  Buffer->ChunkCount = ChunkCount;
  Buffer->ReadIndex = 0;
  Buffer->WriteIndex = 0;
}

void ChunkRingBufferWrite(crb *Buffer, const void *Data, memsize Length) {
  memsize NewWriteIndex = (Buffer->WriteIndex + 1) % Buffer->ChunkCount;
  Assert(NewWriteIndex != Buffer->ReadIndex);

  memsize ReadOffset = Buffer->Offsets[Buffer->ReadIndex];
  memsize WriteOffset = Buffer->Offsets[Buffer->WriteIndex];
  if(ReadOffset <= WriteOffset) {
    memsize Capacity = Buffer->DataCapacity - WriteOffset;
    if(Length > Capacity) {
      Assert(Length <= ReadOffset);
      Buffer->Offsets[Buffer->WriteIndex] = WriteOffset = 0;
    }
  }
  else {
    memsize Capacity = ReadOffset - WriteOffset;
    Assert(Length <= Capacity);
  }

  Buffer->Sizes[Buffer->WriteIndex] = Length;
  void *Destination = ((ui8*)Buffer->Data) + WriteOffset;
  memcpy(Destination, Data, Length);
  Buffer->Offsets[NewWriteIndex] = WriteOffset + Length;

  MemoryBarrier;

  Buffer->WriteIndex = NewWriteIndex;
}

memsize ChunkRingBufferRead(crb *Buffer, void **Chunk) {
  if(Buffer->ReadIndex == Buffer->WriteIndex) {
    return 0;
  }
  memsize ReadOffset = Buffer->Offsets[Buffer->ReadIndex];
  *Chunk = ((ui8*)Buffer->Data) + ReadOffset;
  memsize Size = Buffer->Sizes[Buffer->ReadIndex];
  Buffer->ReadIndex = (Buffer->ReadIndex + 1) % Buffer->ChunkCount;
  return Size;
}

void TerminateChunkRingBuffer(crb *Buffer) {
  Buffer->WriteIndex = 0;
  Buffer->ChunkCount = 0;
  Buffer->Offsets = NULL;
  Buffer->Sizes = NULL;
  Buffer->Data = NULL;
  Buffer->DataCapacity = 0;
  Buffer->ReadIndex = 0;
}
