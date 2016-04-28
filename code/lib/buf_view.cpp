#include <string.h>
#include "lib/assert.h"
#include "buf_view.h"

buf_view CreateBufView(buffer Buffer) {
  buf_view S;
  S.Position = 0;
  S.Buffer = Buffer;
  return S;
}

void BufViewWrite(buf_view *V, const void *Data, memsize Length) {
  Assert(Length <= V->Buffer.Length - V->Position);
  void *Destination = ((ui8*)V->Buffer.Addr) + V->Position;
  memcpy(Destination, Data, Length);
  V->Position += Length;
}

void BufViewWriteMemsize(buf_view *V, memsize Size) {
  BufViewWrite(V, &Size, sizeof(Size));
}

void BufViewWriteBuffer(buf_view *V, buffer Buffer) {
  BufViewWrite(V, Buffer.Addr, Buffer.Length);
}

void BufViewWriteUI8(buf_view *View, ui8 Int) {
  BufViewWrite(View, &Int, sizeof(Int));
}

void BufViewWriteUI16(buf_view *View, ui16 Int) {
  BufViewWrite(View, &Int, sizeof(Int));
}

void BufViewWriteSI16(buf_view *View, si16 Int) {
  BufViewWrite(View, &Int, sizeof(Int));
}

void* BufViewRead(buf_view *V, memsize Length) {
  void *Result = (ui8*)V->Buffer.Addr + V->Position;
  V->Position += Length;
  return Result;
}

memsize GetRemainingSize(buf_view *V) {
  return V->Buffer.Length - V->Position;
}

ui8 BufViewReadUI8(buf_view *V) {
  ui8 Int = *(ui8*)BufViewRead(V, sizeof(ui8));
  return Int;
}

ui16 BufViewReadUI16(buf_view *V) {
  ui16 Int = *(ui16*)BufViewRead(V, sizeof(ui16));
  return Int;
}

si16 BufViewReadSI16(buf_view *V) {
  si16 Int = *(si16*)BufViewRead(V, sizeof(si16));
  return Int;
}

memsize BufViewReadMemsize(buf_view *V) {
  memsize Size = *(memsize*)BufViewRead(V, sizeof(memsize));
  return Size;
}
