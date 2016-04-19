#pragma once

#include "lib/def.h"

struct chunk_list {
  buffer Buffer;
  memsize ReadPos;
  memsize WritePos;
};

void InitChunkList(chunk_list *List, buffer Data);
void ResetChunkList(chunk_list *List);
void ChunkListWrite(chunk_list *List, buffer Chunk);
memsize ChunkListRead(chunk_list *List, buffer Chunk);
void TerminateChunkList(chunk_list *List);
