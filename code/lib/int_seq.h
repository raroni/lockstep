#pragma once

#include "lib/def.h"

struct int_seq {
  memsize *Ints;
  memsize Count;
  memsize Capacity;
  memsize WritePos;
};

void InitIntSeq(int_seq *Seq, memsize *Ints, memsize Capacity);
void IntSeqPush(int_seq *Seq, memsize Int);
double CalcIntSeqStdDev(int_seq *Seq);
void TerminateIntSeq(int_seq *Seq);
