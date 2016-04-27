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

simulation_player_id SimulationCreatePlayer(simulation *Sim) {
  Assert(Sim->PlayerCount != SIMULATION_PLAYER_MAX);
  ui16 Displacement = 200;
  player *Player = Sim->Players + Sim->PlayerCount;
  Player->ID = Sim->PlayerCount;
  for(memsize U=0; U<UNITS_PER_PLAYER; ++U) {
    CreateUnit(Sim, Player->ID, U*Displacement, U*Displacement/2 + Player->ID * 200);
  }
  Sim->PlayerCount++;
  return Player->ID;
}

void InitSimulation(simulation *Sim) {
  Sim->UnitCount = 0;
  Sim->PlayerCount = 0;
}

void TickSimulation(simulation *Sim, order_list *OrderList) {
  if(OrderList->Count != 0) {
    printf("Got order:\n");
    for(memsize I=0; I<OrderList->Count; ++I) {
      printf("PlayerID: %d, UnitCount: %d, x: %d, y: %d\n", OrderList->Orders[I].PlayerID, OrderList->Orders[I].UnitCount, OrderList->Orders[I].Target.X, OrderList->Orders[I].Target.Y);
      for(memsize N=0; N<OrderList->Orders[I].UnitCount; ++N) {
        printf("... UnitID: %d\n", OrderList->Orders[I].UnitIDs[N]);
      }
    }
  }
}
