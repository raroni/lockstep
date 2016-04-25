#pragma once

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
buffer ChunkRingBufferPeek(chunk_ring_buffer *Buffer);
void ChunkRingBufferReadAdvance(chunk_ring_buffer *Buffer);
memsize GetChunkRingBufferUnreadCount(chunk_ring_buffer *Buffer);
void TerminateChunkRingBuffer(chunk_ring_buffer *Buffer);
