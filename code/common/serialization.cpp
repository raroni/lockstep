#include <string.h>
#include "lib/assert.h"
#include "serialization.h"

serializer CreateSerializer(buffer Buffer) {
  serializer S;
  S.Position = 0;
  S.Buffer = Buffer;
  return S;
}

void ResetSerializer(serializer *Serializer) {
  Serializer->Position = 0;
}

void SerializerWrite(serializer *S, const void *Data, memsize Length) {
  Assert(Length <= S->Buffer.Length - S->Position);
  void *Destination = ((ui8*)S->Buffer.Addr) + S->Position;
  memcpy(Destination, Data, Length);
  S->Position += Length;
}

void SerializerWriteMemsize(serializer *S, memsize Size) {
  SerializerWrite(S, &Size, sizeof(Size));
}

void SerializerWriteBuffer(serializer *S, buffer Buffer) {
  SerializerWrite(S, Buffer.Addr, Buffer.Length);
}

void SerializerWriteUI8(serializer *Serializer, ui8 Int) {
  SerializerWrite(Serializer, &Int, sizeof(Int));
}

void* SerializerRead(serializer *S, memsize Length) {
  void *Result = (ui8*)S->Buffer.Addr + S->Position;
  S->Position += Length;
  return Result;
}

memsize GetRemainingSize(serializer *S) {
  return S->Buffer.Length - S->Position;
}

ui8 SerializerReadUI8(serializer *S) {
  ui8 Int = *(ui8*)SerializerRead(S, sizeof(ui8));
  return Int;
}

memsize SerializerReadMemsize(serializer *S) {
  memsize Size = *(memsize*)SerializerRead(S, sizeof(memsize));
  return Size;
}
