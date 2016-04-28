#include "lib/def.h"
#include "common/memory.h"

struct seq_write {
  linear_allocator *Allocator;
  buffer Buffer;
};

seq_write CreateSeqWrite(linear_allocator *Allocator);
void SeqWriteUI8(seq_write *Writer, ui8 Int);
void SeqWriteUI16(seq_write *Writer, ui16 Int);
void SeqWriteSI16(seq_write *Writer, si16 Int);
buffer GetSeqWriteBuffer(seq_write *Writer);
