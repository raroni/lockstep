#pragma once

#include "lib/def.h"
#include "lib/math.h"

#define SIMULATION_PLAYER_MAX 8
#define SIMULATION_UNIT_MAX 4096
#define SIMULATION_TREE_COUNT 24
#define SIMULATION_UNIT_HALF_SIZE 50
#define SIMULATION_TREE_HALF_SIZE 50
#define SIMULATION_UNDEFINED_PLAYER_ID SIMULATION_PLAYER_MAX
#define SIMULATION_UNDEFINED_UNIT_ID SIMULATION_PLAYER_MAX

typedef ui8 simulation_player_id;
typedef ui16 simulation_unit_id;

const umsec32 SimulationTickDuration = 100;

struct simulation_order {
  simulation_player_id PlayerID;
  simulation_unit_id *UnitIDs;
  ui16 UnitCount;
  ivec2 Target;
};

struct simulation_order_list {
  simulation_order *Orders;
  ui16 Count;
};

struct simulation_unit {
  simulation_player_id PlayerID;
  simulation_unit_id ID;
  ivec2 Pos;
  ivec2 Target;
};

struct simulation_tree {
  ivec2 Pos;
};

struct simulation_player {
  simulation_player_id ID;
};

struct simulation {
  simulation_unit Units[SIMULATION_UNIT_MAX];
  simulation_tree Trees[SIMULATION_TREE_COUNT];
  simulation_player Players[SIMULATION_PLAYER_MAX];
  ui16 UnitCount;
  ui8 PlayerCount;
};

void InitSimulation(simulation *Sim);
simulation_player_id SimulationCreatePlayer(simulation *Sim);
void TickSimulation(simulation *Sim, simulation_order_list *OrderList);
