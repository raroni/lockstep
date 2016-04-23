#include "simulation.h"

#define UNITS_PER_PLAYER 4

typedef simulation_unit unit;

void CreateUnit(simulation *Sim, memsize PlayerID, ui16 X, ui16 Y) {
  unit *Unit = Sim->Units + Sim->UnitCount;

  Unit->PlayerID = PlayerID;
  Unit->X = X;
  Unit->Y = Y;

  Sim->UnitCount++;
}

void InitSimulation(simulation *Sim, memsize PlayerCount) {
  Sim->UnitCount = 0;

  ui16 Displacement = 200;
  for(memsize I=0; I<PlayerCount; ++I) {
    for(memsize U=0; U<UNITS_PER_PLAYER; ++U) {
      CreateUnit(Sim, I, U*Displacement, 0);
    }
  }
}

void AdvanceSimulation(simulation *Sim) {

}
