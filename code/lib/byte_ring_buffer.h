#pragma once

#include "def.h"

struct byte_ring_buffer {
  buffer Storage;
  _Atomic memsize ReadPos;
  _Atomic memsize WritePos;
};

void InitByteRingBuffer(byte_ring_buffer *Buffer, buffer Storage);
void ByteRingBufferWrite(byte_ring_buffer *Buffer, buffer Input);
memsize ByteRingBufferRead(byte_ring_buffer *Buffer, buffer Output);
memsize ByteRingBufferPeek(byte_ring_buffer *Buffer, buffer Output);
void ByteRingBufferReadAdvance(byte_ring_buffer *Buffer, memsize Length);
memsize ByteRingBufferCalcFree(byte_ring_buffer *Buffer);
void TerminateByteRingBuffer(byte_ring_buffer *Buffer);

/* Not thead safe */
void ByteRingBufferReset(byte_ring_buffer *Buffer);
