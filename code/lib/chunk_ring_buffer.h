#ifndef CHUNK_RING_BUFFER_H
#define CHUNK_RING_BUFFER_H

#include "def.h"

struct chunk_ring_buffer {
  memsize WriteIndex;
  memsize ChunkCount;
  memsize *Offsets;
  memsize *Sizes;
  buffer Data;
  memsize ReadIndex;
};

void InitChunkRingBuffer(
  chunk_ring_buffer *Buffer,
  memsize ChunkCount,
  buffer Storage
);
void ChunkRingBufferWrite(chunk_ring_buffer *Buffer, const buffer Input);
memsize ChunkRingBufferRead(chunk_ring_buffer *Buffer, const buffer Output);
void TerminateChunkRingBuffer(chunk_ring_buffer *Buffer);

#endif
