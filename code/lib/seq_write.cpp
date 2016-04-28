#include <string.h>
#include "seq_write.h"

seq_write CreateSeqWrite(memory_arena *Arena) {
  seq_write Writer;

  Writer.Arena = Arena;
  Writer.Buffer.Addr = GetMemoryArenaHead(Arena);
  Writer.Buffer.Length = 0;
  return Writer;
}

void SeqWrite(seq_write *Writer, const void *DataAddr, memsize DataLength) {
  void *Destination = MemoryArenaAllocate(Writer->Arena, DataLength);
  memcpy(Destination, DataAddr, DataLength);
  Writer->Buffer.Length += DataLength;
}

void SeqWriteMemsize(seq_write *Writer, memsize Int) {
  SeqWrite(Writer, &Int, sizeof(Int));
}

void SeqWriteBuffer(seq_write *Writer, buffer Buffer) {
  SeqWrite(Writer, Buffer.Addr, Buffer.Length);
}

void SeqWriteUI8(seq_write *Writer, ui8 Int) {
  SeqWrite(Writer, &Int, sizeof(Int));
}

void SeqWriteUI16(seq_write *Writer, ui16 Int) {
  SeqWrite(Writer, &Int, sizeof(Int));
}

void SeqWriteSI16(seq_write *Writer, si16 Int) {
  SeqWrite(Writer, &Int, sizeof(Int));
}
