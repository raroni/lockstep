#include <string.h>
#include "lib/assert.h"
#include "common/serialization.h"
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

memsize ChunkListRead(chunk_list *List, buffer Chunk) {
  if(List->ReadPos == List->WritePos) {
    return 0;
  }
  buffer WriteBuffer = GetSubBuffer(List, List->ReadPos);
  serializer S = CreateSerializer(WriteBuffer);
  memsize Length = SerializerReadMemsize(&S);
  Assert(Length <= Chunk.Length);
  void *Source = SerializerRead(&S, Length);
  memcpy(Chunk.Addr, Source, Length);

  List->ReadPos += S.Position;

  return Length;
}

void TerminateChunkList(chunk_list *List) {
  ResetChunkList(List);
  List->Buffer.Addr = NULL;
  List->Buffer.Length = 0;
}
