#include "lib/def.h"
#include "common/simulation.h"

struct interpolation_sample_pair {
  ivec2 Old;
  ivec2 Current;
};

struct interpolation {
  interpolation_sample_pair SamplePairs[SIMULATION_UNIT_MAX];
  rvec2 Pos[SIMULATION_UNIT_MAX];
  memsize Count;
};

void InitInterpolation(interpolation *Interpolation, simulation *Sim);
void ReloadInterpolation(interpolation *Interpolation, simulation *Sim);
void UpdateInterpolation(interpolation *Interpolation, simulation *Sim, r32 TickProgress);
