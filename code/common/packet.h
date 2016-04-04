#ifndef PACKET_H
#define PACKET_H

#include <stddef.h>
#include "shared.h"

struct packet {
  void *Data;
  size_t Length;
  size_t Capacity;
};

void PacketWriteUI8(packet *Packet, ui8 Int);
void ResetPacket(packet *Packet);

#endif
