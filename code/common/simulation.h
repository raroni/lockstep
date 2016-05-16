#pragma once

#include "lib/def.h"
#include "lib/math.h"
#include "lib/memory_arena.h"

#define SIMULATION_PLAYER_MAX 8
#define SIMULATION_UNIT_MAX 4096
#define SIMULATION_TREE_COUNT 18
#define SIMULATION_UNIT_HALF_SIZE 10
#define SIMULATION_TREE_HALF_SIZE 50
#define SIMULATION_ENTITY_MAX_SIZE 50
#define SIMULATION_UNDEFINED_PLAYER_ID SIMULATION_PLAYER_MAX
#define SIMULATION_UNDEFINED_UNIT_ID SIMULATION_PLAYER_MAX
#define SIMULATION_UNDEFINED_BODY_ID UINT16_MAX
#define SIMULATION_WIDTH 2000
#define SIMULATION_HEIGHT 2000
#define SIMULATION_HALF_WIDTH (SIMULATION_WIDTH/2)
#define SIMULATION_HALF_HEIGHT (SIMULATION_HEIGHT/2)
#define SIMULATION_CELL_SIZE 100
#define SIMULATION_GRID_WIDTH ((SIMULATION_WIDTH + SIMULATION_CELL_SIZE - 1) / SIMULATION_CELL_SIZE)
#define SIMULATION_GRID_HEIGHT ((SIMULATION_HEIGHT + SIMULATION_CELL_SIZE - 1) / SIMULATION_CELL_SIZE)
#define SIMULATION_CELL_COUNT (SIMULATION_GRID_WIDTH * SIMULATION_GRID_HEIGHT)

typedef ui8 simulation_player_id;
typedef ui16 simulation_unit_id;
typedef ui16 simulation_body_id;

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
  simulation_body_id BodyID;
  simulation_unit_id ID;
  ivec2 Target;
};

struct simulation_body_cell_node {
  simulation_body_id ID;
  simulation_body_cell_node *Next;
};

struct simulation_body_cell {
  simulation_body_cell_node *First;
};

struct simulation_body_list {
  ivec2 *Poss;
  ui16 Count;
  ui16 Max;
  simulation_body_cell Cells[SIMULATION_CELL_COUNT];
  simulation_body_cell_node *FirstFreeNode;
  simulation_body_cell_node *CellNodes;
};

struct simulation_player {
  simulation_player_id ID;
};

struct simulation {
  simulation_body_list DynamicBodyList;
  simulation_body_list StaticBodyList;
  simulation_unit Units[SIMULATION_UNIT_MAX];
  simulation_player Players[SIMULATION_PLAYER_MAX];
  ui16 UnitCount;
  ui8 PlayerCount;
};

void InitSimulation(simulation *Sim, memory_arena *Arena);
memsize SimulationFindUnits(simulation *Sim, irect Rect, simulation_unit_id *IDs, memsize Max);
simulation_player_id SimulationCreatePlayer(simulation *Sim);
void TickSimulation(simulation *Sim, simulation_order_list *OrderList);
ivec2 SimulationGetUnitPos(simulation *Sim, simulation_unit *Unit);
