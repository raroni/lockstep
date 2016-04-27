#include "lib/def.h"
#include "common/simulation.h"

struct interpolation {
  ivec2 Positions[SIMULATION_UNIT_MAX];
  memsize Count;
};

void InitInterpolation(interpolation *Interpolation, simulation *Sim);
void UpdateInterpolation(interpolation *Interpolation, simulation *Sim);
