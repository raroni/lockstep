#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "shared.h"

struct ring_buffer {
  memsize ReadPos;
  void *Data;
  memsize Capacity;
  memsize WritePos;
};

void InitRingBuffer(ring_buffer *Buffer, void *Data, memsize Capacity);
void RingBufferWrite(ring_buffer *Buffer, const void *Data, memsize Length);
memsize RingBufferRead(ring_buffer *Buffer, void *ReadBuffer, memsize MaxLength);
memsize RingBufferCalcFree(ring_buffer *Buffer);
void TerminateRingBuffer(ring_buffer *Buffer);

#endif
