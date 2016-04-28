#include "lib/assert.h"
#include "interpolation.h"

typedef simulation_unit sim_unit;

static void CreateUnit(interpolation *Interpolation, sim_unit *Units, memsize I) {
  sim_unit *SimUnit = Units + I;

  Interpolation->SamplePairs[I].Old = SimUnit->Pos;
  Interpolation->SamplePairs[I].Current = SimUnit->Pos;
  Interpolation->Pos[I] = ConvertIvec2ToRvec2(SimUnit->Pos);

  Interpolation->Count++;
}

void InitInterpolation(interpolation *Interpolation, simulation *Sim) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    CreateUnit(Interpolation, Sim->Units, I);
  }
}

void ReloadInterpolation(interpolation *Interpolation, simulation *Sim) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    Interpolation->SamplePairs[I].Old = Interpolation->SamplePairs[I].Current;
    Interpolation->SamplePairs[I].Current = Sim->Units[I].Pos;
  }
}

void UpdateInterpolation(interpolation *Interpolation, simulation *Sim, r32 TickProgress) {
  Assert(TickProgress >= 0 && TickProgress <= 1);
  for(memsize I=0; I<Interpolation->Count; ++I) {
    interpolation_sample_pair *Samples = Interpolation->SamplePairs + I;
    rvec2 Dif = ConvertIvec2ToRvec2(Samples->Current - Samples->Old);
    Interpolation->Pos[I] = ConvertIvec2ToRvec2(Samples->Old) + Dif * TickProgress;
  }
}
