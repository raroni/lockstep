#include "orwell.h"
#include "../code/lib/ring_buffer.h"

static void TestBasicWriteRead(ow_test_context Context) {
  ui8 RingBufferData[16];
  ring_buffer Buffer;
  InitRingBuffer(&Buffer, RingBufferData, sizeof(RingBufferData));
  RingBufferWrite(&Buffer, "hey", 4);

  ui8 ReadBuffer[4];
  memsize ReadLength = RingBufferRead(&Buffer, &ReadBuffer, sizeof(ReadBuffer));

  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualStr("hey", (const char*)(ReadBuffer));

  TerminateRingBuffer(&Buffer);
}

static void TestWrappingWriteRead(ow_test_context Context) {
  ui8 RingBufferData[8];
  ui8 ReadBuffer[8];
  ring_buffer Buffer;
  InitRingBuffer(&Buffer, RingBufferData, sizeof(RingBufferData));


  ui8 Input1[] = { 1, 2, 3, 4 };
  RingBufferWrite(&Buffer, Input1, sizeof(Input1));
  memsize ReadLength = RingBufferRead(&Buffer, ReadBuffer, sizeof(ReadBuffer));
  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualInt(1, ReadBuffer[0]);
  OW_AssertEqualInt(2, ReadBuffer[1]);
  OW_AssertEqualInt(3, ReadBuffer[2]);
  OW_AssertEqualInt(4, ReadBuffer[3]);

  ui8 Input2[] = { 10, 11, 12, 13, 14, 15 };
  RingBufferWrite(&Buffer, Input2, sizeof(Input2));
  ReadLength = RingBufferRead(&Buffer, ReadBuffer, sizeof(ReadBuffer));
  OW_AssertEqualInt(6, ReadLength);
  OW_AssertEqualInt(10, ReadBuffer[0]);
  OW_AssertEqualInt(11, ReadBuffer[1]);
  OW_AssertEqualInt(12, ReadBuffer[2]);
  OW_AssertEqualInt(13, ReadBuffer[3]);
  OW_AssertEqualInt(14, ReadBuffer[4]);
  OW_AssertEqualInt(15, ReadBuffer[5]);

  TerminateRingBuffer(&Buffer);
}

static void TestBasicCalcFree(ow_test_context Context) {
  ui8 RingBufferData[8];
  ui8 DummyBuffer[8] = { };
  ring_buffer Buffer;
  InitRingBuffer(&Buffer, RingBufferData, sizeof(RingBufferData));
  RingBufferWrite(&Buffer, DummyBuffer, 4);
  OW_AssertEqualInt(3, RingBufferCalcFree(&Buffer));
  TerminateRingBuffer(&Buffer);
}

static void TestWrappingCalcFree(ow_test_context Context) {
  ui8 RingBufferData[8];
  ui8 DummyBuffer[8] = { };
  ring_buffer Buffer;
  InitRingBuffer(&Buffer, RingBufferData, sizeof(RingBufferData));
  RingBufferWrite(&Buffer, DummyBuffer, 4);
  RingBufferRead(&Buffer, DummyBuffer, sizeof(DummyBuffer));
  RingBufferWrite(&Buffer, DummyBuffer, 6);
  OW_AssertEqualInt(1, RingBufferCalcFree(&Buffer));
  TerminateRingBuffer(&Buffer);
}

void SetupRingBufferGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasicWriteRead);
  OW_AddTest(S, G, TestWrappingWriteRead);
  OW_AddTest(S, G, TestBasicCalcFree);
  OW_AddTest(S, G, TestWrappingCalcFree);
}
