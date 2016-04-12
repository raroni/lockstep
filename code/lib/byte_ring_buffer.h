#pragma once

#include "def.h"

struct byte_ring_buffer {
  buffer Storage;
  memsize ReadPos;
  memsize WritePos;
};

void InitByteRingBuffer(byte_ring_buffer *Buffer, buffer Storage);
void ByteRingBufferWrite(byte_ring_buffer *Buffer, buffer Input);
memsize ByteRingBufferRead(byte_ring_buffer *Buffer, buffer Output);
memsize ByteRingBufferCalcFree(byte_ring_buffer *Buffer);
void TerminateByteRingBuffer(byte_ring_buffer *Buffer);
