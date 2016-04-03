#include "orwell.h"
#include "../code/lib/byte_ring_buffer.h"

static void TestBasicWriteRead(ow_test_context Context) {
  ui8 RingBufferData[16];
  byte_ring_buffer Buffer;
  InitByteRingBuffer(&Buffer, RingBufferData, sizeof(RingBufferData));
  ByteRingBufferWrite(&Buffer, "hey", 4);

  ui8 ReadBuffer[4];
  memsize ReadLength = ByteRingBufferRead(&Buffer, &ReadBuffer, sizeof(ReadBuffer));

  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualStr("hey", (const char*)(ReadBuffer));

  TerminateByteRingBuffer(&Buffer);
}

static void TestWrappingWriteRead(ow_test_context Context) {
  ui8 RingBufferData[8];
  ui8 ReadBuffer[8];
  byte_ring_buffer Buffer;
  InitByteRingBuffer(&Buffer, RingBufferData, sizeof(RingBufferData));


  ui8 Input1[] = { 1, 2, 3, 4 };
  ByteRingBufferWrite(&Buffer, Input1, sizeof(Input1));
  memsize ReadLength = ByteRingBufferRead(&Buffer, ReadBuffer, sizeof(ReadBuffer));
  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualInt(1, ReadBuffer[0]);
  OW_AssertEqualInt(2, ReadBuffer[1]);
  OW_AssertEqualInt(3, ReadBuffer[2]);
  OW_AssertEqualInt(4, ReadBuffer[3]);

  ui8 Input2[] = { 10, 11, 12, 13, 14, 15 };
  ByteRingBufferWrite(&Buffer, Input2, sizeof(Input2));
  ReadLength = ByteRingBufferRead(&Buffer, ReadBuffer, sizeof(ReadBuffer));
  OW_AssertEqualInt(6, ReadLength);
  OW_AssertEqualInt(10, ReadBuffer[0]);
  OW_AssertEqualInt(11, ReadBuffer[1]);
  OW_AssertEqualInt(12, ReadBuffer[2]);
  OW_AssertEqualInt(13, ReadBuffer[3]);
  OW_AssertEqualInt(14, ReadBuffer[4]);
  OW_AssertEqualInt(15, ReadBuffer[5]);

  TerminateByteRingBuffer(&Buffer);
}

static void TestBasicCalcFree(ow_test_context Context) {
  ui8 RingBufferData[8];
  ui8 DummyBuffer[8] = { };
  byte_ring_buffer Buffer;
  InitByteRingBuffer(&Buffer, RingBufferData, sizeof(RingBufferData));
  ByteRingBufferWrite(&Buffer, DummyBuffer, 4);
  OW_AssertEqualInt(3, ByteRingBufferCalcFree(&Buffer));
  TerminateByteRingBuffer(&Buffer);
}

static void TestWrappingCalcFree(ow_test_context Context) {
  ui8 RingBufferData[8];
  ui8 DummyBuffer[8] = { };
  byte_ring_buffer Buffer;
  InitByteRingBuffer(&Buffer, RingBufferData, sizeof(RingBufferData));
  ByteRingBufferWrite(&Buffer, DummyBuffer, 4);
  ByteRingBufferRead(&Buffer, DummyBuffer, sizeof(DummyBuffer));
  ByteRingBufferWrite(&Buffer, DummyBuffer, 6);
  OW_AssertEqualInt(1, ByteRingBufferCalcFree(&Buffer));
  TerminateByteRingBuffer(&Buffer);
}

static void TestEmpty(ow_test_context Context) {
  ui8 RingBufferData[8];
  byte_ring_buffer Buffer;
  InitByteRingBuffer(&Buffer, RingBufferData, sizeof(RingBufferData));
  ui8 Input[4];
  ByteRingBufferWrite(&Buffer, Input, 4);

  ui8 Output[2];
  memsize OutputLength = ByteRingBufferRead(&Buffer, Output, sizeof(Output));
  OW_AssertEqualInt(2, OutputLength);
  OutputLength = ByteRingBufferRead(&Buffer, Output, sizeof(Output));
  OW_AssertEqualInt(2, OutputLength);
  OutputLength = ByteRingBufferRead(&Buffer, Output, sizeof(Output));
  OW_AssertEqualInt(0, OutputLength);
  OutputLength = ByteRingBufferRead(&Buffer, Output, sizeof(Output));
  OW_AssertEqualInt(0, OutputLength);

  TerminateByteRingBuffer(&Buffer);
}

void SetupByteRingBufferGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasicWriteRead);
  OW_AddTest(S, G, TestWrappingWriteRead);
  OW_AddTest(S, G, TestBasicCalcFree);
  OW_AddTest(S, G, TestWrappingCalcFree);
  OW_AddTest(S, G, TestEmpty);
}
