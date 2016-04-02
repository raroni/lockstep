#include <string.h>
#include "orwell.h"
#include "../code/lib/chunk_ring_buffer.h"

static void TestBasicWriteRead(ow_test_context Context) {
  ui8 RingBufferData[256];
  chunk_ring_buffer Buffer;
  InitChunkRingBuffer(&Buffer, 2, RingBufferData, sizeof(RingBufferData));
  ChunkRingBufferWrite(&Buffer, "hey", 4);

  void *Result;
  memsize ReadLength = ChunkRingBufferRead(&Buffer, &Result);

  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualStr("hey", (const char*)(Result));

  TerminateChunkRingBuffer(&Buffer);
}

static void TestLoopingWriteRead(ow_test_context Context) {
  const char *Words[] = { "hello", "what", "is", "up", "overthere" };
  ui8 WordCount = sizeof(Words)/sizeof(char*);
  ui8 RingBufferData[256];
  chunk_ring_buffer Buffer;
  InitChunkRingBuffer(&Buffer, 5, RingBufferData, sizeof(RingBufferData));
  ChunkRingBufferWrite(&Buffer, Words[0], strlen(Words[0])+1);
  ChunkRingBufferWrite(&Buffer, Words[1], strlen(Words[1])+1);
  memsize ReadIndex = 0;
  memsize WriteIndex = 2;

  for(memsize I=0; I<200; I++) {
    void *Result;
    memsize ReadLength = ChunkRingBufferRead(&Buffer, &Result);
    OW_AssertEqualInt(strlen(Words[ReadIndex])+1, ReadLength);
    OW_AssertEqualStr(Words[ReadIndex], (const char*)(Result));

    ChunkRingBufferWrite(&Buffer, Words[WriteIndex], strlen(Words[WriteIndex])+1);

    ReadIndex = (ReadIndex + 1) % WordCount;
    WriteIndex = (WriteIndex + 1) % WordCount;
  }

  TerminateChunkRingBuffer(&Buffer);
}

void SetupChunkRingBufferGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasicWriteRead);
  OW_AddTest(S, G, TestLoopingWriteRead);
}
