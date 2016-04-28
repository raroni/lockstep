#include "lib/assert.h"
#include "lib/math.h"
#include "simulation.h"

#define UNITS_PER_PLAYER 4

typedef simulation_unit unit;
typedef simulation_player_id player_id;
typedef simulation_order_list order_list;
typedef simulation_player player;

void CreateUnit(simulation *Sim, memsize PlayerID, ivec2 Pos) {
  unit *Unit = Sim->Units + Sim->UnitCount;

  Unit->ID = Sim->UnitCount;
  Unit->PlayerID = PlayerID;
  Unit->Pos = Pos;
  Unit->Target = Pos;

  Sim->UnitCount++;
}

simulation_player_id SimulationCreatePlayer(simulation *Sim) {
  Assert(Sim->PlayerCount != SIMULATION_PLAYER_MAX);
  ui16 Displacement = 200;
  player *Player = Sim->Players + Sim->PlayerCount;
  Player->ID = Sim->PlayerCount;
  for(memsize U=0; U<UNITS_PER_PLAYER; ++U) {
    ivec2 Pos = MakeIvec2(U*Displacement, U*Displacement/2 + Player->ID * 200);
    CreateUnit(Sim, Player->ID, Pos);
  }
  Sim->PlayerCount++;
  return Player->ID;
}

static void UpdateUnits(simulation *Sim) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    simulation_unit *Unit = Sim->Units + I;
    rvec2 Pos = ConvertIvec2ToRvec2(Unit->Pos);
    rvec2 Target = ConvertIvec2ToRvec2(Unit->Target);
    rvec2 Difference = Target - Pos;
    r32 SquaredDistance = Difference.X * Difference.X + Difference.Y * Difference.Y;
    if(SquaredDistance > 0.01) {
      r32 Distance = SquareRoot(SquaredDistance);
      rvec2 Direction = Difference / Distance;
      static const r32 Speed = .1;
      rvec2 PositionChange = Direction * Speed * SimulationTickDuration;
      PositionChange = ClampRvec2(PositionChange, Distance);
      rvec2 NewPos = Pos + PositionChange;
      Unit->Pos = ConvertRvec2ToIvec2(NewPos);
    }
  }
}

void InitSimulation(simulation *Sim) {
  Sim->UnitCount = 0;
  Sim->PlayerCount = 0;
}

void TickSimulation(simulation *Sim, order_list *OrderList) {
  for(memsize I=0; I<OrderList->Count; ++I) {
    simulation_order *Order = OrderList->Orders + I;
    for(memsize N=0; N<Order->UnitCount; ++N) {
      simulation_unit *Unit = Sim->Units + Order->UnitIDs[N];
      if(Unit->PlayerID == Order->PlayerID) {
        Unit->Target = Order->Target;
      }
    }
  }
  UpdateUnits(Sim);
}
