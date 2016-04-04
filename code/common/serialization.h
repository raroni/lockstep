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
void SerializerWriteUI8(serializer *Serializer, ui8 Int);
void ResetSerializer(serializer *Serializer);

#endif
