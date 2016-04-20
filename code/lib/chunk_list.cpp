#include "lib/assert.h"
#include "lib/serialization.h"
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
}

void ChunkListWrite(chunk_list *List, buffer Chunk) {
  buffer WriteBuffer = GetSubBuffer(List, List->WritePos);
  serializer S = CreateSerializer(WriteBuffer);
  SerializerWriteMemsize(&S, Chunk.Length);
  SerializerWriteBuffer(&S, Chunk);
  List->WritePos += S.Position;
}

buffer ChunkListRead(chunk_list *List) {
  buffer Result;
  if(List->ReadPos != List->WritePos) {
    buffer ReadBuffer = GetSubBuffer(List, List->ReadPos);
    serializer S = CreateSerializer(ReadBuffer);
    memsize Length = SerializerReadMemsize(&S);

    Result.Addr = (ui8*)ReadBuffer.Addr + S.Position;
    Result.Length = Length;

    List->ReadPos += S.Position + Length;
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
