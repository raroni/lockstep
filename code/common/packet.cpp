#include <string.h>
#include "lib/assert.h"
#include "packet.h"

typedef packet_cursor cursor;

cursor CreatePacketCursor(void *Data, memsize Capacity) {
  cursor C;
  C.Position = 0;
  C.Data = Data;
  C.Capacity = Capacity;
  return C;
}

void PacketWrite(cursor *C, void *Data, memsize Length) {
  Assert(Length <= C->Capacity - C->Position);
  void *Destination = ((ui8*)C->Data) + C->Position;
  memcpy(Destination, Data, Length);
  C->Position += Length;
}

void PacketWriteUI8(cursor *Cursor, ui8 Int) {
  PacketWrite(Cursor, &Int, sizeof(Int));
}

void ResetPacketCursor(cursor *Cursor) {
  Cursor->Position = 0;
}
