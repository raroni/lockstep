#include "common/conversion.h"
#include "serialization.h"
#include "messages.h"

enum message_type {
  message_type_start = 123 // Temp dummy value
};

memsize SerializeStartMessage(buffer Buffer) {
  ui8 TypeInt = SafeCastIntToUI8(message_type_start);
  serializer Writer = CreateSerializer(Buffer);
  SerializerWriteUI8(&Writer, TypeInt);
  return Writer.Position;
}
