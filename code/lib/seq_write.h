#include "lib/def.h"
#include "lib/memory.h"

struct seq_write {
  linear_allocator *Allocator;
  buffer Buffer;
};

seq_write CreateSeqWrite(linear_allocator *Allocator);
void SeqWrite(seq_write *Writer, const void *DataAddr, memsize DataLength);
void SeqWriteUI8(seq_write *Writer, ui8 Int);
void SeqWriteUI16(seq_write *Writer, ui16 Int);
void SeqWriteSI16(seq_write *Writer, si16 Int);
void SeqWriteBuffer(seq_write *Writer, buffer Buffer);
void SeqWriteMemsize(seq_write *Writer, memsize Int);
buffer GetSeqWriteBuffer(seq_write *Writer);
