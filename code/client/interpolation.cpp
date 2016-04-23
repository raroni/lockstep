#include "interpolation.h"

typedef simulation_unit sim_unit;

static void CreateUnit(interpolation *Interpolation, sim_unit *Units, memsize I) {
  sim_unit *SimUnit = Units + I;

  Interpolation->Positions[I].X = SimUnit->X;
  Interpolation->Positions[I].Y = SimUnit->Y;

  Interpolation->Count++;
}

void InitInterpolation(interpolation *Interpolation, simulation *Sim) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    CreateUnit(Interpolation, Sim->Units, I);
  }
}
