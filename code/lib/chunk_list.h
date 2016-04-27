#pragma once

#include "lib/def.h"

struct chunk_list {
  buffer Buffer;
  memsize ReadPos;
  memsize WritePos;
  memsize Count;
};

void InitChunkList(chunk_list *List, buffer Data);
void ResetChunkList(chunk_list *List);
void* ChunkListAllocate(chunk_list *List, memsize Length);
void ChunkListWrite(chunk_list *List, buffer Chunk);
buffer ChunkListRead(chunk_list *List);
void TerminateChunkList(chunk_list *List);
memsize GetChunkListCount(chunk_list *List);
