#include "int_seq.h"

void InitIntSeq(int_seq *Seq, memsize *Ints, memsize Capacity) {
  Seq->Count = 0;
  Seq->WritePos = 0;
  Seq->Capacity = Capacity;
  Seq->Ints = Ints;
}

void IntSeqPush(int_seq *Seq, memsize Int) {
  Seq->Ints[Seq->WritePos] = Int;
  Seq->WritePos = (Seq->WritePos + 1) % Seq->Capacity;
  if(Seq->Count != Seq->Capacity) {
    Seq->Count++;
  }
}

double CalcIntSeqVariance(int_seq *Seq) {
  if(Seq->Count == 0) {
    return 0;
  }

  double IntSum = 0;
  for(memsize I=0; I<Seq->Count; ++I) {
    IntSum += Seq->Ints[I];
  }
  double IntMean = IntSum / Seq->Count;

  double SquaredDevSum = 0;
  for(memsize I=0; I<Seq->Count; ++I) {
    double Dev = Seq->Ints[I] - IntMean;
    SquaredDevSum += Dev*Dev;
  }
  double Variance = SquaredDevSum / Seq->Count;

  return Variance;
}

void TerminateIntSeq(int_seq *Seq) {
  Seq->Count = 0;
  Seq->WritePos = 0;
  Seq->Capacity = 0;
  Seq->Ints = NULL;
}
