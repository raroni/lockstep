#include <string.h>
#include "lib/assert.h"
#include "serialization.h"

serializer CreateSerializer(void *Data, memsize Capacity) {
  serializer S;
  S.Position = 0;
  S.Data = Data;
  S.Capacity = Capacity;
  return S;
}

void SerializerWrite(serializer *S, void *Data, memsize Length) {
  Assert(Length <= S->Capacity - S->Position);
  void *Destination = ((ui8*)S->Data) + S->Position;
  memcpy(Destination, Data, Length);
  S->Position += Length;
}

void SerializerWriteUI8(serializer *Serializer, ui8 Int) {
  SerializerWrite(Serializer, &Int, sizeof(Int));
}

void ResetSerializer(serializer *Serializer) {
  Serializer->Position = 0;
}
