#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "def.h"

struct byte_ring_buffer {
  memsize ReadPos;
  void *Data;
  memsize Capacity;
  memsize WritePos;
};

void InitByteRingBuffer(byte_ring_buffer *Buffer, void *Data, memsize Capacity);
void ByteRingBufferWrite(byte_ring_buffer *Buffer, const void *Data, memsize Length);
memsize ByteRingBufferRead(byte_ring_buffer *Buffer, void *ReadBuffer, memsize MaxLength);
memsize ByteRingBufferCalcFree(byte_ring_buffer *Buffer);
void TerminateByteRingBuffer(byte_ring_buffer *Buffer);

#endif
