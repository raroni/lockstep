#include "lib/def.h"
#include "common/simulation.h"
#include "common/config.h"

struct interpolation_position {
  r32 X;
  r32 Y;
};

struct interpolation {
  interpolation_position Positions[UNIT_MAX];
  memsize Count;
};

void InitInterpolation(interpolation *Interpolation, simulation *Sim);
