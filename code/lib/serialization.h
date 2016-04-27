#pragma once

#include <stddef.h>
#include "lib/def.h"

struct serializer {
  size_t Position;
  buffer Buffer;
};

serializer CreateSerializer(buffer Buffer);
void ResetSerializer(serializer *Serializer);
memsize GetRemainingSize(serializer *S);

void SerializerWrite(serializer *S, const void *Data, memsize Length);
void SerializerWriteMemsize(serializer *S, memsize Size);
void SerializerWriteBuffer(serializer *S, buffer Buffer);
void SerializerWriteUI8(serializer *Serializer, ui8 Int);
void SerializerWriteUI16(serializer *Serializer, ui16 Int);
void SerializerWriteSI16(serializer *Serializer, si16 Int);

void* SerializerRead(serializer *S, memsize Length);
ui8 SerializerReadUI8(serializer *Serializer);
ui16 SerializerReadUI16(serializer *Serializer);
si16 SerializerReadSI16(serializer *Serializer);
memsize SerializerReadMemsize(serializer *S);
