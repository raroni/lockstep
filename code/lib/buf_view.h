#pragma once

#include <stddef.h>
#include "lib/def.h"

struct buf_view {
  size_t Position;
  buffer Buffer;
};

buf_view CreateBufView(buffer Buffer);
memsize GetRemainingSize(buf_view *V);

void BufViewWrite(buf_view *V, const void *Data, memsize Length);
void BufViewWriteMemsize(buf_view *V, memsize Size);
void BufViewWriteBuffer(buf_view *V, buffer Buffer);
void BufViewWriteUI8(buf_view *View, ui8 Int);
void BufViewWriteUI16(buf_view *View, ui16 Int);
void BufViewWriteSI16(buf_view *View, si16 Int);

void* BufViewRead(buf_view *View, memsize Length);
ui8 BufViewReadUI8(buf_view *View);
ui16 BufViewReadUI16(buf_view *View);
si16 BufViewReadSI16(buf_view *View);
memsize BufViewReadMemsize(buf_view *View);
