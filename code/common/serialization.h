#pragma once

#include <stddef.h>
#include "lib/def.h"

struct serializer {
  size_t Position;
  buffer Buffer;
};

serializer CreateSerializer(buffer Buffer);
void ResetSerializer(serializer *Serializer);

void SerializerWrite(serializer *S, const void *Data, memsize Length);
void SerializerWriteMemsize(serializer *S, memsize Size);
void SerializerWriteBuffer(serializer *S, buffer Buffer);
void SerializerWriteUI8(serializer *Serializer, ui8 Int);

void* SerializerRead(serializer *S, memsize Length);
ui8 SerializerReadUI8(serializer *Serializer);
memsize SerializerReadMemsize(serializer *S);
