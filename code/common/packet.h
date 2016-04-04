#ifndef PACKET_H
#define PACKET_H

#include <stddef.h>
#include "shared.h"

struct packet_cursor {
  size_t Position;
  size_t Capacity;
  void *Data;
};

packet_cursor CreatePacketCursor(void *Data, memsize Capacity);
void PacketWriteUI8(packet_cursor *Cursor, ui8 Int);
void ResetPacketCursor(packet_cursor *Cursor);

#endif
