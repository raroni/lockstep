#include <string.h>
#include "orwell.h"
#include "lib/byte_ring_buffer.h"

static void TestBasicWriteRead(ow_test_context Context) {
  ui8 RingStorageBlock[16];
  buffer Storage = { .Addr = &RingStorageBlock, .Length = sizeof(RingStorageBlock) };
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);

  char InputBlock[4];
  strcpy(InputBlock, "hey");
  buffer Input = { .Addr = InputBlock, .Length = 4 };
  ByteRingBufferWrite(&Ring, Input);

  ui8 OutputBlock[4];
  buffer Output = { .Addr = OutputBlock, .Length = sizeof(OutputBlock) };
  memsize ReadLength = ByteRingBufferRead(&Ring, Output);

  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualStr("hey", (const char*)(Output.Addr));

  TerminateByteRingBuffer(&Ring);
}

static void TestWrappingWriteRead(ow_test_context Context) {
  ui8 RingStorageBlock[8];
  buffer Storage = { .Addr = &RingStorageBlock, .Length = sizeof(RingStorageBlock) };
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
}

static void TestBasicCalcFree(ow_test_context Context) {
  ui8 RingStorageBlock[8];
  buffer Storage = { .Addr = &RingStorageBlock, .Length = sizeof(RingStorageBlock) };
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);
  ui8 DummyBuffer[4] = { };
  buffer Input = { .Addr = DummyBuffer, .Length = sizeof(DummyBuffer) };
  ByteRingBufferWrite(&Ring, Input);
  OW_AssertEqualInt(3, ByteRingBufferCalcFree(&Ring));
  TerminateByteRingBuffer(&Ring);
}

static void TestWrappingCalcFree(ow_test_context Context) {
  ui8 RingStorageBlock[8];
  buffer Storage = { .Addr = &RingStorageBlock, .Length = sizeof(RingStorageBlock) };
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
}

static void TestEmpty(ow_test_context Context) {
  ui8 RingStorageBlock[8];
  buffer Storage = { .Addr = &RingStorageBlock, .Length = sizeof(RingStorageBlock) };
  byte_ring_buffer Ring;
  InitByteRingBuffer(&Ring, Storage);
  ui8 InputBlock[4];
  buffer Input = { .Addr = InputBlock, .Length = sizeof(InputBlock) };
  ByteRingBufferWrite(&Ring, Input);

  ui8 OutputBlock[2];
  buffer Output = { .Addr = OutputBlock, .Length = sizeof(OutputBlock) };
  memsize OutputLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(2, OutputLength);
  OutputLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(2, OutputLength);
  OutputLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(0, OutputLength);
  OutputLength = ByteRingBufferRead(&Ring, Output);
  OW_AssertEqualInt(0, OutputLength);

  TerminateByteRingBuffer(&Ring);
}

void SetupByteRingBufferGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasicWriteRead);
  OW_AddTest(S, G, TestWrappingWriteRead);
  OW_AddTest(S, G, TestBasicCalcFree);
  OW_AddTest(S, G, TestWrappingCalcFree);
  OW_AddTest(S, G, TestEmpty);
}
