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

  buffer Chunk = ChunkListRead(&List);
  OW_AssertEqualInt(3, Chunk.Length);
  OW_AssertEqualInt(3, ((ui8*)Chunk.Addr)[0]);
  OW_AssertEqualInt(2, ((ui8*)Chunk.Addr)[1]);
  OW_AssertEqualInt(1, ((ui8*)Chunk.Addr)[2]);

  Chunk = ChunkListRead(&List);
  OW_AssertEqualInt(4, Chunk.Length);
  OW_AssertEqualInt(4, ((ui8*)Chunk.Addr)[0]);
  OW_AssertEqualInt(5, ((ui8*)Chunk.Addr)[1]);
  OW_AssertEqualInt(6, ((ui8*)Chunk.Addr)[2]);
  OW_AssertEqualInt(7, ((ui8*)Chunk.Addr)[3]);

  Chunk = ChunkListRead(&List);
  OW_AssertEqualInt(0, Chunk.Length);

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

  buffer Chunk = ChunkListRead(&List);
  OW_AssertEqualInt(0, Chunk.Length);

  DestroyTestBuffer(&IOBuffer);
  TerminateChunkList(&List);
  DestroyTestBuffer(&BackBuffer);
}

static void TestAllocate(ow_test_context Context) {
  chunk_list List;
  buffer BackBuffer = CreateTestBuffer(64);
  InitChunkList(&List, BackBuffer);

  ui64 *InputUI64 = (ui64*)ChunkListAllocate(&List, sizeof(ui64));
  *InputUI64 = 12345;

  buffer Chunk = ChunkListRead(&List);
  OW_AssertEqualInt(8, Chunk.Length);
  ui64 OutputUI64 = *(ui64*)Chunk.Addr;
  OW_AssertEqualInt(12345, OutputUI64);

  TerminateChunkList(&List);
  DestroyTestBuffer(&BackBuffer);
}

void SetupChunkListGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasicWriteRead);
  OW_AddTest(S, G, TestReset);
  OW_AddTest(S, G, TestAllocate);
}
