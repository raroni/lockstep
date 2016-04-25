#include <string.h>
#include "orwell.h"
#include "lib/chunk_ring_buffer.h"

static void TestBasicWriteRead(ow_test_context Context) {
  ui8 RingBufferBlock[256];
  buffer RingBuffer = {
    .Addr = &RingBufferBlock,
    .Length = sizeof(RingBufferBlock)
  };
  chunk_ring_buffer Ring;
  InitChunkRingBuffer(&Ring, 2, RingBuffer);
  char InputBlock[] = { 'h', 'e', 'y', '\0' };
  const buffer Input = { .Addr = &InputBlock, .Length = 4 };
  ChunkRingBufferWrite(&Ring, Input);

  char OutputBlock[8];
  buffer Output = { .Addr = &OutputBlock, .Length = sizeof(OutputBlock) };
  memsize ReadLength = ChunkRingBufferRead(&Ring, Output);

  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualStr("hey", OutputBlock);

  TerminateChunkRingBuffer(&Ring);
}

static void TestLoopingWriteRead(ow_test_context Context) {
  char InputBufferBlocks[5][10];
  strcpy(InputBufferBlocks[0], "hello");
  strcpy(InputBufferBlocks[1], "what");
  strcpy(InputBufferBlocks[2], "is");
  strcpy(InputBufferBlocks[3], "up");
  strcpy(InputBufferBlocks[4], "overthere");

  buffer Inputs[] = {
    { .Addr = InputBufferBlocks[0], .Length = strlen(InputBufferBlocks[0])+1 },
    { .Addr = InputBufferBlocks[1], .Length = strlen(InputBufferBlocks[1])+1 },
    { .Addr = InputBufferBlocks[2], .Length = strlen(InputBufferBlocks[2])+1 },
    { .Addr = InputBufferBlocks[3], .Length = strlen(InputBufferBlocks[3])+1 },
  };
  ui8 InputCount = sizeof(Inputs)/sizeof(Inputs[0]);

  ui8 RingBufferBlock[256];
  buffer RingBuffer = {
    .Addr = &RingBufferBlock,
    .Length = sizeof(RingBufferBlock)
  };
  chunk_ring_buffer Ring;
  InitChunkRingBuffer(&Ring, 5, RingBuffer);
  ChunkRingBufferWrite(&Ring, Inputs[0]);
  ChunkRingBufferWrite(&Ring, Inputs[1]);
  memsize ReadIndex = 0;
  memsize WriteIndex = 2;

  for(memsize I=0; I<200; I++) {
    char ResultBuffer[10];
    buffer Result = { .Addr = &ResultBuffer, .Length = sizeof(ResultBuffer) };
    memsize ReadLength = ChunkRingBufferRead(&Ring, Result);
    OW_AssertEqualInt(Inputs[ReadIndex].Length, ReadLength);
    OW_AssertEqualStr((const char*)Inputs[ReadIndex].Addr, (const char*)Result.Addr);

    ChunkRingBufferWrite(&Ring, Inputs[WriteIndex]);

    ReadIndex = (ReadIndex + 1) % InputCount;
    WriteIndex = (WriteIndex + 1) % InputCount;
  }

  TerminateChunkRingBuffer(&Ring);
}

static void TestEmpty(ow_test_context Context) {
  ui8 RingBufferBlock[128];
  buffer RingBuffer = { .Addr = RingBufferBlock, .Length = sizeof(RingBufferBlock) };
  chunk_ring_buffer Ring;
  InitChunkRingBuffer(&Ring, 4, RingBuffer);
  ui8 InputBlock[4];
  buffer Input = { .Addr = InputBlock, .Length = sizeof(InputBlock) };
  ChunkRingBufferWrite(&Ring, Input);

  char ResultBlock[8];
  buffer Result = { .Addr = ResultBlock, .Length = sizeof(ResultBlock) };
  memsize ReadLength = ChunkRingBufferRead(&Ring, Result);
  OW_AssertEqualInt(4, ReadLength);
  ReadLength = ChunkRingBufferRead(&Ring, Result);
  OW_AssertEqualInt(0, ReadLength);
  ReadLength = ChunkRingBufferRead(&Ring, Result);
  OW_AssertEqualInt(0, ReadLength);

  TerminateChunkRingBuffer(&Ring);
}

static void TestPeek(ow_test_context Context) {
  ui8 RingBufferBlock[256];
  buffer RingBuffer = { .Addr = &RingBufferBlock, .Length = sizeof(RingBufferBlock) };
  chunk_ring_buffer Ring;
  InitChunkRingBuffer(&Ring, 2, RingBuffer);

  char InputBlock[] = { 'h', 'e', 'y', '\0' };
  buffer Input = { .Addr = &InputBlock, .Length = sizeof(InputBlock) };
  ChunkRingBufferWrite(&Ring, Input);

  buffer Peek = ChunkRingBufferPeek(&Ring);
  OW_AssertEqualInt(4, Peek.Length);
  OW_AssertEqualStr("hey", (const char*)Peek.Addr);

  TerminateChunkRingBuffer(&Ring);
}

static void TestReadAdvance(ow_test_context Context) {
  ui8 RingBufferBlock[256];
  buffer RingBuffer = { .Addr = &RingBufferBlock, .Length = sizeof(RingBufferBlock) };
  chunk_ring_buffer Ring;
  InitChunkRingBuffer(&Ring, 4, RingBuffer);

  char Input1Block[] = { 'h', 'e', 'y', '\0' };
  buffer Input1 = { .Addr = &Input1Block, .Length = sizeof(Input1Block) };
  char Input2Block[] = { 'y', 'o', 'u', '\0' };
  buffer Input2 = { .Addr = &Input2Block, .Length = sizeof(Input2Block) };

  ChunkRingBufferWrite(&Ring, Input1);
  ChunkRingBufferWrite(&Ring, Input2);

  ChunkRingBufferReadAdvance(&Ring);

  buffer Peek = ChunkRingBufferPeek(&Ring);
  OW_AssertEqualInt(4, Peek.Length);
  OW_AssertEqualStr("you", (const char*)Peek.Addr);

  TerminateChunkRingBuffer(&Ring);
}

void SetupChunkRingBufferGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasicWriteRead);
  OW_AddTest(S, G, TestLoopingWriteRead);
  OW_AddTest(S, G, TestPeek);
  OW_AddTest(S, G, TestReadAdvance);
  OW_AddTest(S, G, TestEmpty);
}
