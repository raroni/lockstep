#include "lib/def.h"
#include "common/memory.h"

struct data_writer {
  linear_allocator *Allocator;
  buffer Buffer;
};

data_writer CreateDataWriter(linear_allocator *Allocator);
void WriteUI8(data_writer *Writer, ui8 Int);
void WriteUI16(data_writer *Writer, ui16 Int);
void WriteSI16(data_writer *Writer, si16 Int);
buffer GetDataWriterBuffer(data_writer *Writer);
