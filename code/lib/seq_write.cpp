#include <string.h>
#include "seq_write.h"

seq_write CreateSeqWrite(linear_allocator *Allocator) {
  seq_write Writer;

  Writer.Allocator = Allocator;
  Writer.Buffer.Addr = GetLinearAllocatorHead(Allocator);
  Writer.Buffer.Length = 0;
  return Writer;
}

void SeqWrite(seq_write *Writer, void *DataAddr, memsize DataLength) {
  void *Destination = LinearAllocate(Writer->Allocator, DataLength);
  memcpy(Destination, DataAddr, DataLength);
  Writer->Buffer.Length += DataLength;
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
