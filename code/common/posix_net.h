#pragma once

#include "lib/def.h"

#define POSIX_PACKET_HEADER_SIZE 2

ssize_t PosixNetReceive(int FD, buffer Message);
buffer PosixExtractPacketMessage(buffer Incoming);
void PosixNetSendPacket(int FD, buffer Message);
