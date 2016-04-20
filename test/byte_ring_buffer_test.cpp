#include <string.h>
#include <stdlib.h>
#include "orwell.h"
#include "lib/byte_ring_buffer.h"

static buffer CreateTestBuffer(memsize Length) {
  void *Addr = malloc(Length);
  buffer Buffer;
  Buffer.Addr = Addr;
  Buffer.Length = Length;
  return Buffer;
}

static void DestroyTestBuffer(buffer Buffer) {
  free(Buffer.Addr);
}

static void TestBasicWriteRead(ow_test_context Context) {
  buffer Storage = CreateTestBuffer(16);
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);

  buffer Input = CreateTestBuffer(4);
  strcpy((char*)Input.Addr, "hey");
  ByteRingBufferWrite(&Ring, Input);

  buffer Output = CreateTestBuffer(4);
  memsize ReadLength = ByteRingBufferRead(&Ring, Output);

  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualStr("hey", (const char*)(Output.Addr));

  DestroyTestBuffer(Output);
  DestroyTestBuffer(Input);
  TerminateByteRingBuffer(&Ring);
  DestroyTestBuffer(Storage);
}

static void TestWrappingWriteRead(ow_test_context Context) {
  buffer Storage = CreateTestBuffer(8);
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);
  ui8 OutputBlock[8];
  buffer Output = { .Addr = OutputBlock, .Length = sizeof(OutputBlock) };

  ui8 Input1Block[] = { 1, 2, 3, 4 };
  buffer Input1 = { .Addr = Input1Block, .Length = sizeof(Input1Block) };
  ByteRingBufferWrite(&Ring, Input1);
  memsize ReadLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualInt(1, OutputBlock[0]);
  OW_AssertEqualInt(2, OutputBlock[1]);
  OW_AssertEqualInt(3, OutputBlock[2]);
  OW_AssertEqualInt(4, OutputBlock[3]);

  ui8 Input2Block[] = { 10, 11, 12, 13, 14, 15 };
  buffer Input2 = { .Addr = Input2Block, .Length = sizeof(Input2Block) };
  ByteRingBufferWrite(&Ring, Input2);
  ReadLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(6, ReadLength);
  OW_AssertEqualInt(10, OutputBlock[0]);
  OW_AssertEqualInt(11, OutputBlock[1]);
  OW_AssertEqualInt(12, OutputBlock[2]);
  OW_AssertEqualInt(13, OutputBlock[3]);
  OW_AssertEqualInt(14, OutputBlock[4]);
  OW_AssertEqualInt(15, OutputBlock[5]);

  TerminateByteRingBuffer(&Ring);
  DestroyTestBuffer(Storage);
}

static void TestBasicCalcFree(ow_test_context Context) {
  buffer Storage = CreateTestBuffer(8);
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);
  buffer Input = CreateTestBuffer(4);
  ByteRingBufferWrite(&Ring, Input);
  DestroyTestBuffer(Input);
  OW_AssertEqualInt(3, ByteRingBufferCalcFree(&Ring));
  TerminateByteRingBuffer(&Ring);
  DestroyTestBuffer(Storage);
}

static void TestWrappingCalcFree(ow_test_context Context) {
  buffer Storage = CreateTestBuffer(8);
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);
  ui8 DummyBuffer[8] = { };
  buffer Input1 = { .Addr = DummyBuffer, .Length = 4 };
  ByteRingBufferWrite(&Ring, Input1);
  buffer Output = { .Addr = DummyBuffer, .Length = 4 };
  ByteRingBufferRead(&Ring, Output);
  buffer Input2 = { .Addr = DummyBuffer, .Length = 6 };
  ByteRingBufferWrite(&Ring, Input2);
  OW_AssertEqualInt(1, ByteRingBufferCalcFree(&Ring));
  TerminateByteRingBuffer(&Ring);
  DestroyTestBuffer(Storage);
}

static void TestEmpty(ow_test_context Context) {
  buffer Storage = CreateTestBuffer(8);
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);
  buffer Input = CreateTestBuffer(4);
  ByteRingBufferWrite(&Ring, Input);

  buffer Output = CreateTestBuffer(2);
  memsize OutputLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(2, OutputLength);
  OutputLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(2, OutputLength);
  OutputLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(0, OutputLength);
  OutputLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(0, OutputLength);

  DestroyTestBuffer(Output);
  DestroyTestBuffer(Input);
  TerminateByteRingBuffer(&Ring);
  DestroyTestBuffer(Storage);
}

static void TestPeek(ow_test_context Context) {
  buffer Storage = CreateTestBuffer(8);
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);

  ui8 DummyBlock[] = { 1, 2, 3, 4 };
  buffer Dummy = { .Addr = DummyBlock, .Length = sizeof(DummyBlock) };
  ByteRingBufferWrite(&Ring, Dummy);
  memsize ReadLength = ByteRingBufferPeek(&Ring, Dummy);
  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualInt(1, DummyBlock[0]);
  OW_AssertEqualInt(2, DummyBlock[1]);
  OW_AssertEqualInt(3, DummyBlock[2]);
  OW_AssertEqualInt(4, DummyBlock[3]);
  OW_AssertEqualInt(0, Ring.ReadPos);

  TerminateByteRingBuffer(&Ring);
  DestroyTestBuffer(Storage);
}

static void TestReadAdvance(ow_test_context Context) {
  buffer Storage = CreateTestBuffer(8);
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);

  ui8 InputBlock[] = { 1, 2, 3, 4 };
  buffer Input = { .Addr = InputBlock, .Length = sizeof(InputBlock) };
  ByteRingBufferWrite(&Ring, Input);
  OW_AssertEqualInt(0, Ring.ReadPos);
  ByteRingBufferReadAdvance(&Ring, 2);
  OW_AssertEqualInt(2, Ring.ReadPos);

  TerminateByteRingBuffer(&Ring);
  DestroyTestBuffer(Storage);
}

static void TestReset(ow_test_context Context) {
  buffer Storage = CreateTestBuffer(8);
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);

  ui8 DummyBlock[] = { 1, 2, 3, 4 };
  buffer Input = { .Addr = DummyBlock, .Length = sizeof(DummyBlock) };
  buffer Output = { .Addr = DummyBlock, .Length = sizeof(DummyBlock)/2 };
  ByteRingBufferWrite(&Ring, Input);
  ByteRingBufferRead(&Ring, Output);
  ByteRingBufferReset(&Ring);
  OW_AssertEqualInt(0, Ring.ReadPos);
  OW_AssertEqualInt(0, Ring.WritePos);

  TerminateByteRingBuffer(&Ring);
  DestroyTestBuffer(Storage);
}

void SetupByteRingBufferGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasicWriteRead);
  OW_AddTest(S, G, TestWrappingWriteRead);
  OW_AddTest(S, G, TestBasicCalcFree);
  OW_AddTest(S, G, TestWrappingCalcFree);
  OW_AddTest(S, G, TestEmpty);
  OW_AddTest(S, G, TestPeek);
  OW_AddTest(S, G, TestReadAdvance);
  OW_AddTest(S, G, TestReset);
}
