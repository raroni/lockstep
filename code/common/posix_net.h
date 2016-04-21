#pragma once

#include "lib/def.h"

ssize_t PosixNetReceive(int FD, buffer Buffer);
ssize_t PosixNetSend(int FD, buffer Buffer);
