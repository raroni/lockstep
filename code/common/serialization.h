#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <stddef.h>
#include "shared.h"

struct serializer {
  size_t Position;
  size_t Capacity;
  void *Data;
};

serializer CreateSerializer(void *Data, memsize Capacity);
void ResetSerializer(serializer *Serializer);

void SerializerWrite(serializer *S, const void *Data, memsize Length);
void SerializerWriteMemsize(serializer *S, memsize Size);
void SerializerWriteUI8(serializer *Serializer, ui8 Int);

void* SerializerRead(serializer *S, memsize Length);
ui8 SerializerReadUI8(serializer *Serializer);
memsize SerializerReadMemsize(serializer *S);

#endif
