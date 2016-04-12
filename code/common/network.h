#pragma once

#include "lib/def.h"

ssize_t NetworkReceive(int FD, buffer Buffer);
ssize_t NetworkSend(int FD, buffer Buffer);
