#pragma once

#include "lib/def.h"

#define SIMULATION_TICK_DURATION 100

struct simulation_unit {
  memsize PlayerID;
  ui16 X;
  ui16 Y;
};

#define SIMULATION_UNIT_MAX 4096

struct simulation {
  simulation_unit Units[SIMULATION_UNIT_MAX];
  ui16 UnitCount;
};

void InitSimulation(simulation *Sim, memsize PlayerCount);
void TickSimulation(simulation *Sim);
