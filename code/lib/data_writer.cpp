#include <string.h>
#include "data_writer.h"

data_writer CreateDataWriter(linear_allocator *Allocator) {
  data_writer Writer;

  Writer.Allocator = Allocator;
  Writer.Buffer.Addr = GetLinearAllocatorHead(Allocator);
  Writer.Buffer.Length = 0;
  return Writer;
}

static void Write(data_writer *Writer, void *DataAddr, memsize DataLength) {
  void *Destination = LinearAllocate(Writer->Allocator, DataLength);
  memcpy(Destination, DataAddr, DataLength);
  Writer->Buffer.Length += DataLength;
}

void WriteUI8(data_writer *Writer, ui8 Int) {
  Write(Writer, &Int, sizeof(Int));
}

void WriteUI16(data_writer *Writer, ui16 Int) {
  Write(Writer, &Int, sizeof(Int));
}

void WriteSI16(data_writer *Writer, si16 Int) {
  Write(Writer, &Int, sizeof(Int));
}
