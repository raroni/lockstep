#include "lib/assert.h"
#include "simulation.h"

#define UNITS_PER_PLAYER 4

typedef simulation_unit unit;
typedef simulation_player_id player_id;
typedef simulation_order_list order_list;
typedef simulation_player player;

void CreateUnit(simulation *Sim, memsize PlayerID, ui16 X, ui16 Y) {
  unit *Unit = Sim->Units + Sim->UnitCount;

  Unit->ID = Sim->UnitCount;
  Unit->PlayerID = PlayerID;
  Unit->X = X;
  Unit->Y = Y;

  Sim->UnitCount++;
}

void CreatePlayer(simulation *Sim, player_id PlayerID) {

}

simulation_player_id SimulationCreatePlayer(simulation *Sim) {
  Assert(Sim->PlayerCount != SIMULATION_PLAYER_MAX);
  ui16 Displacement = 200;
  player *Player = Sim->Players + Sim->PlayerCount;
  Player->ID = Sim->PlayerCount;
  for(memsize U=0; U<UNITS_PER_PLAYER; ++U) {
    CreateUnit(Sim, Player->ID, U*Displacement, U*Displacement/2);
  }
  Sim->PlayerCount++;
  return Player->ID;
}

void InitSimulation(simulation *Sim) {
  Sim->UnitCount = 0;
  Sim->PlayerCount = 0;
}

void TickSimulation(simulation *Sim, order_list *OrderList) {

}
