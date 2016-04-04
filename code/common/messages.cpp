#include "packet.h"
#include "messages.h"

enum message_type {
  message_type_start = 123 // Temp dummy value
};

memsize SerializeStartMessage(void *Buffer, memsize Length) {
  ui8 TypeInt = SafeCastIntToUI8(message_type_start);
  packet_cursor Writer = CreatePacketCursor(Buffer, Length);
  PacketWriteUI8(&Writer, TypeInt);
  return Writer.Position;
}
