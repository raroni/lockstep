#include "assert.h"
#include "packet.h"

void PacketWriteUI8(packet *Packet, ui8 Int) {
  size_t Length = sizeof(ui8);
  Assert(Packet->Capacity - Packet->Length >= Length);
  ui8 *C = (ui8*)Packet->Data;
  *C = Int;
  Packet->Length += Length;
}

void ResetPacket(packet *Packet) {
  Packet->Length = 0;
}
