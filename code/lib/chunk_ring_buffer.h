#ifndef CHUNK_RING_BUFFER_H
#define CHUNK_RING_BUFFER_H

#include "def.h"

struct chunk_ring_buffer {
  memsize WriteIndex;
  memsize ChunkCount;
  memsize *Offsets;
  memsize *Sizes;
  void *Data;
  memsize DataCapacity;
  memsize ReadIndex;
};

void InitChunkRingBuffer(
  chunk_ring_buffer *Buffer,
  memsize ChunkCount,
  void *Data,
  memsize DataLength
);
void ChunkRingBufferWrite(chunk_ring_buffer *Buffer, const void *Data, memsize Length);
memsize ChunkRingBufferRead(chunk_ring_buffer *Buffer, void **ReadBuffer);
void TerminateChunkRingBuffer(chunk_ring_buffer *Buffer);

#endif
