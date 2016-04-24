#include "orwell.h"
#include "lib/int_seq.h"

#define countof(A) sizeof(A)/sizeof(A[0])

static void TestVarCalcNonFull(ow_test_context Context) {
  int_seq Seq;
  memsize Ints[8];
  InitIntSeq(&Seq, Ints, countof(Ints));

  IntSeqPush(&Seq, 3);
  IntSeqPush(&Seq, 4);
  IntSeqPush(&Seq, 7);
  IntSeqPush(&Seq, 10);

  double Result = CalcIntSeqVariance(&Seq);
  OW_AssertInDelta(7.5, 0.0001, Result);

  TerminateIntSeq(&Seq);
}

static void TestVarCalcWithOverflow(ow_test_context Context) {
  int_seq Seq;
  memsize Ints[3];
  InitIntSeq(&Seq, Ints, countof(Ints));

  IntSeqPush(&Seq, 3);
  IntSeqPush(&Seq, 4);
  IntSeqPush(&Seq, 7);
  IntSeqPush(&Seq, 17);

  double Result = CalcIntSeqVariance(&Seq);
  OW_AssertInDelta(30.888889, 0.0001, Result);

  TerminateIntSeq(&Seq);
}

void SetupIntSeqGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestVarCalcNonFull);
  OW_AddTest(S, G, TestVarCalcWithOverflow);
}
