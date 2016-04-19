#include <stdlib.h>
#include "orwell.h"
#include "lib/def.h"
#include "lib/chunk_list.h"

static buffer CreateTestBuffer(memsize Length) {
  void *Addr = malloc(Length);
  buffer Buffer;
  Buffer.Addr = Addr;
  Buffer.Length = Length;
  return Buffer;
}

static void DestroyTestBuffer(buffer *Buffer) {
  free(Buffer->Addr);
  Buffer->Addr = NULL;
  Buffer->Length = 0;
}

static void TestBasicWriteRead(ow_test_context Context) {
  chunk_list List;
  buffer BackBuffer = CreateTestBuffer(32);
  InitChunkList(&List, BackBuffer);

  ui8 Message1Block[] = { 3, 2, 1 };
  buffer Message1 = { .Addr = &Message1Block, .Length = sizeof(Message1Block) };
  ui8 Message2Block[] = { 4, 5, 6, 7 };
  buffer Message2 = { .Addr = &Message2Block, .Length = sizeof(Message2Block) };
  ChunkListWrite(&List, Message1);
  ChunkListWrite(&List, Message2);

  memsize Length;
  ui8 ReadBufferBlock[16];
  buffer ReadBuffer = { .Addr = &ReadBufferBlock, .Length = sizeof(ReadBufferBlock) };

  Length = ChunkListRead(&List, ReadBuffer);
  OW_AssertEqualInt(3, Length);
  OW_AssertEqualInt(3, ReadBufferBlock[0]);
  OW_AssertEqualInt(2, ReadBufferBlock[1]);
  OW_AssertEqualInt(1, ReadBufferBlock[2]);

  Length = ChunkListRead(&List, ReadBuffer);
  OW_AssertEqualInt(4, Length);
  OW_AssertEqualInt(4, ReadBufferBlock[0]);
  OW_AssertEqualInt(5, ReadBufferBlock[1]);
  OW_AssertEqualInt(6, ReadBufferBlock[2]);
  OW_AssertEqualInt(7, ReadBufferBlock[3]);

  Length = ChunkListRead(&List, ReadBuffer);
  OW_AssertEqualInt(0, Length);

  TerminateChunkList(&List);
  DestroyTestBuffer(&BackBuffer);
}

static void TestReset(ow_test_context Context) {
  chunk_list List;
  buffer BackBuffer = CreateTestBuffer(32);
  InitChunkList(&List, BackBuffer);

  buffer IOBuffer = CreateTestBuffer(4);
  ChunkListWrite(&List, IOBuffer);

  ResetChunkList(&List);

  memsize Length = ChunkListRead(&List, IOBuffer);
  OW_AssertEqualInt(0, Length);

  DestroyTestBuffer(&IOBuffer);
  TerminateChunkList(&List);
  DestroyTestBuffer(&BackBuffer);
}

void SetupChunkListGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasicWriteRead);
  OW_AddTest(S, G, TestReset);
}
