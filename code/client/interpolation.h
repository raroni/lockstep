#include "lib/def.h"
#include "common/simulation.h"

struct interpolation_position {
  r32 X;
  r32 Y;
};

struct interpolation {
  interpolation_position Positions[SIMULATION_UNIT_MAX];
  memsize Count;
};

void InitInterpolation(interpolation *Interpolation, simulation *Sim);
