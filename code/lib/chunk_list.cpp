#include "lib/assert.h"
#include "lib/buf_view.h"
#include "chunk_list.h"

buffer GetSubBuffer(chunk_list *List, memsize Pos) {
  buffer Buffer;
  Buffer.Addr = (ui8*)List->Buffer.Addr + Pos;
  Buffer.Length = List->Buffer.Length - Pos;
  return Buffer;
}

void InitChunkList(chunk_list *List, buffer Buffer) {
  List->Buffer = Buffer;
  ResetChunkList(List);
}

void ResetChunkList(chunk_list *List) {
  List->ReadPos = 0;
  List->WritePos = 0;
  List->Count = 0;
}

void ChunkListWrite(chunk_list *List, buffer Chunk) {
  Assert(List->Buffer.Length - List->WritePos >= Chunk.Length + sizeof(memsize));
  buffer WriteBuffer = GetSubBuffer(List, List->WritePos);
  buf_view V = CreateBufView(WriteBuffer);
  BufViewWriteMemsize(&V, Chunk.Length);
  BufViewWriteBuffer(&V, Chunk);
  List->Count++;
  List->WritePos += V.Position;
}

void* ChunkListAllocate(chunk_list *List, memsize Length) {
  Assert(List->Buffer.Length - List->WritePos >= Length + sizeof(memsize));
  buffer WriteBuffer = GetSubBuffer(List, List->WritePos);
  buf_view V = CreateBufView(WriteBuffer);
  BufViewWriteMemsize(&V, Length);
  void *Result = (ui8*)WriteBuffer.Addr + V.Position;
  List->WritePos += V.Position + Length;
  List->Count++;
  return Result;
}

buffer ChunkListRead(chunk_list *List) {
  buffer Result;
  if(List->ReadPos != List->WritePos) {
    Assert(List->Count != 0);
    buffer ReadBuffer = GetSubBuffer(List, List->ReadPos);
    buf_view V = CreateBufView(ReadBuffer);
    memsize Length = BufViewReadMemsize(&V);

    Result.Addr = (ui8*)ReadBuffer.Addr + V.Position;
    Result.Length = Length;

    List->ReadPos += V.Position + Length;
    List->Count--;
  }
  else {
    Result.Addr = NULL;
    Result.Length = 0;
  }

  return Result;
}

void TerminateChunkList(chunk_list *List) {
  ResetChunkList(List);
  List->Buffer.Addr = NULL;
  List->Buffer.Length = 0;
}
