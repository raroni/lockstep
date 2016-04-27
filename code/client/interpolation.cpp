#include "interpolation.h"

typedef simulation_unit sim_unit;

static void CreateUnit(interpolation *Interpolation, sim_unit *Units, memsize I) {
  sim_unit *SimUnit = Units + I;

  Interpolation->Positions[I] = SimUnit->Pos;

  Interpolation->Count++;
}

void InitInterpolation(interpolation *Interpolation, simulation *Sim) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    CreateUnit(Interpolation, Sim->Units, I);
  }
}

void UpdateInterpolation(interpolation *Interpolation, simulation *Sim) {
  for(memsize I=0; I<Interpolation->Count; ++I) {
    Interpolation->Positions[I] = Sim->Units[I].Pos;
  }
}
