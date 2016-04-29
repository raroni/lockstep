#include "lib/assert.h"
#include "lib/math.h"
#include "simulation.h"

#define UNITS_PER_PLAYER 4

typedef simulation_unit unit;
typedef simulation_player_id player_id;
typedef simulation_order_list order_list;
typedef simulation_player player;
typedef simulation_tree tree;

static const ivec2 UndefinedTarget = { .X = INT16_MIN, .Y = INT16_MIN };
static const ivec2 StartPositions[] = {
  { -700, 500 },
  { 700, -500 },
  { -700, -500 },
  { 700, 500 }
};


void CreateUnit(simulation *Sim, memsize PlayerID, ivec2 Pos) {
  unit *Unit = Sim->Units + Sim->UnitCount;

  Unit->ID = Sim->UnitCount;
  Unit->PlayerID = PlayerID;
  Unit->Pos = Pos;
  Unit->Target = UndefinedTarget;

  Sim->UnitCount++;
}

simulation_player_id SimulationCreatePlayer(simulation *Sim) {
  Assert(Sim->PlayerCount != SIMULATION_PLAYER_MAX);
  ui16 Displacement = 160;
  player *Player = Sim->Players + Sim->PlayerCount;
  Player->ID = Sim->PlayerCount;
  memsize StartPositionCount = sizeof(StartPositions) / sizeof(StartPositions[0]);
  ivec2 Base = StartPositions[Player->ID % StartPositionCount];
  for(memsize U=0; U<UNITS_PER_PLAYER; ++U) {
    ivec2 Pos;
    memsize Layer = U % (UNITS_PER_PLAYER / 2);
    Pos.X = (r32)Base.X + (r32(Layer) - 0.5f) * Displacement;
    Pos.Y = Base.Y + ((r32)(U/2)-0.5f) * Displacement;
    CreateUnit(Sim, Player->ID, Pos);
  }
  Sim->PlayerCount++;
  return Player->ID;
}

static void UpdateUnits(simulation *Sim) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    simulation_unit *Unit = Sim->Units + I;
    if(Unit->Target == UndefinedTarget) {
      continue;
    }
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

  for(memsize I=0; I<SIMULATION_TREE_COUNT; ++I) {
    tree *Tree = Sim->Trees + I;
    memsize LineIndex = I % 6;
    Tree->Pos.X = -750 + LineIndex * 150 + I * 50;
    Tree->Pos.Y = -350 + LineIndex * 150;
  }
}

void PerformCollisions(simulation *Sim) {
  memsize SquaredUnitHalfSize = SIMULATION_UNIT_HALF_SIZE * SIMULATION_UNIT_HALF_SIZE;
  memsize SquaredTreeHalfSize = SIMULATION_TREE_HALF_SIZE * SIMULATION_TREE_HALF_SIZE;

  for(memsize U1=0; U1<Sim->UnitCount; ++U1) {
    for(memsize U2=0; U2<Sim->UnitCount; ++U2) {
      if(U1 == U2) {
        continue;
      }
      rvec2 PosDif = ConvertIvec2ToRvec2(Sim->Units[U1].Pos - Sim->Units[U2].Pos);
      r32 SquaredDistance = CalcRvec2SquaredMagnitude(PosDif);
      if(SquaredDistance < SquaredUnitHalfSize*2) {
        rvec2 HalfPosDif = PosDif * 0.501f;
        Sim->Units[U1].Pos += ConvertRvec2ToIvec2(HalfPosDif);
        Sim->Units[U2].Pos -= ConvertRvec2ToIvec2(HalfPosDif);
      }
    }

    for(memsize T=0; T<SIMULATION_TREE_COUNT; ++T) {
      rvec2 PosDif = ConvertIvec2ToRvec2(Sim->Units[U1].Pos - Sim->Trees[T].Pos);
      r32 SquaredDistance = CalcRvec2SquaredMagnitude(PosDif);
      if(SquaredDistance < SquaredUnitHalfSize + SquaredTreeHalfSize) {
        rvec2 Bounce = PosDif * 1.01;
        Sim->Units[U1].Pos += ConvertRvec2ToIvec2(Bounce);
      }
    }
  }
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
  PerformCollisions(Sim);
}
