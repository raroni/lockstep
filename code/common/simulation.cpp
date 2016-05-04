#include "lib/assert.h"
#include "lib/math.h"
#include "lib/min_max.h"
#include "simulation.h"

#define UNITS_PER_PLAYER 64

typedef simulation_unit unit;
typedef simulation_player_id player_id;
typedef simulation_order_list order_list;
typedef simulation_player player;
typedef simulation_body_cell_node body_cell_node;
typedef simulation_body_id body_id;
typedef simulation_body_list body_list;
typedef simulation_body_cell body_cell;

static const ivec2 UndefinedTarget = { .X = INT16_MIN, .Y = INT16_MIN };
static const ivec2 StartPositions[] = {
  { -700, 500 },
  { 700, -500 },
  { -700, -500 },
  { 700, 500 }
};

static inline ivec2 CalcCellPos(ivec2 SimPos) {
  int PositiveX = SimPos.X + SIMULATION_WIDTH / 2;
  int PositiveY = SimPos.Y + SIMULATION_HEIGHT / 2;

  ivec2 Result;
  Result.X = PositiveX / SIMULATION_CELL_SIZE;
  Result.Y = PositiveY / SIMULATION_CELL_SIZE;

  return Result;
}

static inline ui16 CalcCellIndexByCellPos(ivec2 Pos) {
  return Pos.Y * SIMULATION_GRID_WIDTH + Pos.X;
}

static inline ui16 CalcCellIndexBySimPos(ivec2 Pos) {
  ivec2 CellPos = CalcCellPos(Pos);
  ui16 Result = CalcCellIndexByCellPos(CellPos);
  return Result;
}

static void RemoveIDFromCell(body_list *List, body_cell *Cell, body_id ID) {
  body_cell_node *PrevNode = NULL;
  for(body_cell_node *CurrentNode = Cell->First; CurrentNode; CurrentNode = CurrentNode->Next) {
    if(CurrentNode->ID == ID) {
      if(PrevNode) {
        PrevNode->Next = CurrentNode->Next;
      }
      else {
        Cell->First = CurrentNode->Next;
      }
      CurrentNode->Next = List->FirstFreeNode;
      CurrentNode->ID = SIMULATION_UNDEFINED_BODY_ID;
      List->FirstFreeNode = CurrentNode;
      if(Cell->First == CurrentNode) {
        Cell->First = NULL;
      }
      return;
    }
    PrevNode = CurrentNode;
  }
  InvalidCodePath;
}

static void AddIDToCell(body_list *List, body_cell *Cell, body_id ID) {
  body_cell_node *Node = List->FirstFreeNode;
  Assert(Node != NULL);
  List->FirstFreeNode = List->FirstFreeNode->Next;

  Node->ID = ID;
  Node->Next = Cell->First;
  Cell->First = Node;
}

static body_id CreateBody(simulation_body_list *List, ivec2 Pos) {
  body_id ID = List->Count;

  List->Poss[ID] = Pos;
  ui16 CellIndex = CalcCellIndexBySimPos(Pos);
  body_cell *Cell = List->Cells + CellIndex;

  AddIDToCell(List, Cell, ID);

  List->Count++;

  return ID;
}

static void CreateUnit(simulation *Sim, memsize PlayerID, ivec2 Pos) {
  unit *Unit = Sim->Units + Sim->UnitCount;

  Unit->ID = Sim->UnitCount;
  Unit->PlayerID = PlayerID;
  Unit->BodyID = CreateBody(&Sim->DynamicBodyList, Pos);
  Unit->Target = UndefinedTarget;

  Sim->UnitCount++;
}

simulation_player_id SimulationCreatePlayer(simulation *Sim) {
  Assert(Sim->PlayerCount != SIMULATION_PLAYER_MAX);
  ui16 Displacement = 5;
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

static ivec2 GetBodyPos(body_list *List, body_id ID) {
  return List->Poss[ID];
}

static ivec2 ClampSimPos(ivec2 Pos) {
  Pos.X = ClampInt(Pos.X, -SIMULATION_HALF_WIDTH, SIMULATION_HALF_WIDTH - 1);
  Pos.Y = ClampInt(Pos.Y, -SIMULATION_HALF_HEIGHT, SIMULATION_HALF_HEIGHT - 1);
  return Pos;
}

static void SetBodyPosition(body_list *List, body_id ID, ivec2 Pos) {
  Pos = ClampSimPos(Pos);

  ui16 OldCellIndex = CalcCellIndexBySimPos(List->Poss[ID]);
  ui16 NewCellIndex = CalcCellIndexBySimPos(Pos);
  if(OldCellIndex != NewCellIndex) {
    body_cell *OldCell = List->Cells + OldCellIndex;
    body_cell *NewCell = List->Cells + NewCellIndex;
    RemoveIDFromCell(List, OldCell, ID);
    AddIDToCell(List, NewCell, ID);
  }
  List->Poss[ID] = Pos;
}

static void UpdateUnits(simulation *Sim) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    simulation_unit *Unit = Sim->Units + I;
    if(Unit->Target == UndefinedTarget) {
      continue;
    }
    rvec2 Pos = ConvertIvec2ToRvec2(GetBodyPos(&Sim->DynamicBodyList, Unit->BodyID));
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
      SetBodyPosition(&Sim->DynamicBodyList, Unit->BodyID, ConvertRvec2ToIvec2(NewPos));
    }
  }
}

void InitBodyList(simulation_body_list *List, ui16 Max, memory_arena *Arena) {
  List->Poss = (ivec2*)MemoryArenaAllocate(Arena, sizeof(ivec2)*Max);
  List->CellNodes = (simulation_body_cell_node*)MemoryArenaAllocate(Arena, sizeof(simulation_body_cell_node)*Max);
  List->Count = 0;
  List->Max = Max;

  for(memsize I=0; I<SIMULATION_CELL_COUNT; ++I) {
    List->Cells[I].First = NULL;
  }

  {
    List->FirstFreeNode = List->CellNodes;
    for(memsize I=0; I<Max-1; ++I) {
      List->CellNodes[I].Next = List->CellNodes + I + 1;
      List->CellNodes[I].ID = SIMULATION_UNDEFINED_BODY_ID;
    }
    List->CellNodes[Max-1].Next = NULL;
    List->CellNodes[Max-1].ID = SIMULATION_UNDEFINED_BODY_ID;
  }

}

void InitSimulation(simulation *Sim, memory_arena *Arena) {
  Sim->UnitCount = 0;
  Sim->PlayerCount = 0;

  InitBodyList(&Sim->DynamicBodyList, SIMULATION_UNIT_MAX, Arena);
  InitBodyList(&Sim->StaticBodyList, SIMULATION_TREE_COUNT, Arena);

  for(memsize I=0; I<SIMULATION_TREE_COUNT; ++I) {
    ivec2 Pos;
    memsize LineIndex = I % 6;
    Pos.X = -750 + LineIndex * 150 + I * 50;
    Pos.Y = -350 + LineIndex * 150;
    CreateBody(&Sim->StaticBodyList, Pos);
  }
}

void PerformCollisions(simulation *Sim) {
  r32 TreeUnitDistanceMin = SIMULATION_TREE_HALF_SIZE + SIMULATION_UNIT_HALF_SIZE;
  r32 SquaredTreeUnitDistanceMin = TreeUnitDistanceMin * TreeUnitDistanceMin;

  r32 UnitUnitDistanceMin = SIMULATION_UNIT_HALF_SIZE * 2;
  r32 SquaredUnitUnitDistanceMin = UnitUnitDistanceMin * UnitUnitDistanceMin;

  for(memsize U1=0; U1<Sim->UnitCount; ++U1) {
    unit *CurrentUnit = Sim->Units + U1;
    memsize RetryCount = 0;
    Retry:
    if(RetryCount++ < 4) {
      ivec2 CurrentUnitPos = GetBodyPos(&Sim->DynamicBodyList, CurrentUnit->BodyID);

      ivec2 CellPosMin, CellPosMax;
      {
        ivec2 SimPosMin;
        SimPosMin.X = CurrentUnitPos.X - SIMULATION_ENTITY_MAX_SIZE;
        SimPosMin.Y = CurrentUnitPos.Y - SIMULATION_ENTITY_MAX_SIZE;
        SimPosMin = ClampSimPos(SimPosMin);
        CellPosMin = CalcCellPos(SimPosMin);

        ivec2 SimPosMax;
        SimPosMax.X = CurrentUnitPos.X + SIMULATION_ENTITY_MAX_SIZE;
        SimPosMax.Y = CurrentUnitPos.Y + SIMULATION_ENTITY_MAX_SIZE;
        SimPosMax = ClampSimPos(SimPosMax);
        CellPosMax = CalcCellPos(SimPosMax);
      }

      for(memsize CellY=CellPosMin.Y; CellY<=CellPosMax.Y; ++CellY) {
        for(memsize CellX=CellPosMin.X; CellX<=CellPosMax.X; ++CellX) {
          ivec2 CellPos = MakeIvec2(CellX, CellY);
          ui16 CellIndex = CalcCellIndexByCellPos(CellPos);
          body_cell *Cell = Sim->DynamicBodyList.Cells + CellIndex;
          if(Cell->First) {
            for(body_cell_node *Node = Cell->First; Node; Node = Node->Next) {
              if(Node->ID == CurrentUnit->BodyID) {
                continue;
              }
              ivec2 OtherUnitPos = GetBodyPos(&Sim->DynamicBodyList, Node->ID);
              rvec2 PosDif = ConvertIvec2ToRvec2(CurrentUnitPos - OtherUnitPos);
              r32 SquaredDistance = CalcRvec2SquaredMagnitude(PosDif);
              if(SquaredDistance < SquaredUnitUnitDistanceMin) {
                r32 Distance = SquareRoot(SquaredDistance);
                rvec2 Direction = PosDif / Distance;
                r32 Overlap = SIMULATION_UNIT_HALF_SIZE + SIMULATION_TREE_HALF_SIZE - Distance;
                ivec2 Bounce = ConvertRvec2ToIvec2(Direction * Overlap * 0.501f);
                SetBodyPosition(&Sim->DynamicBodyList, CurrentUnit->BodyID, CurrentUnitPos + Bounce);
                SetBodyPosition(&Sim->DynamicBodyList, Node->ID, OtherUnitPos - Bounce);
                goto Retry;
              }
            }
          }

          Cell = Sim->StaticBodyList.Cells + CellIndex;
          if(Cell->First) {
            for(body_cell_node *Node = Cell->First; Node; Node = Node->Next) {
              if(Node->ID == CurrentUnit->BodyID) {
                continue;
              }
              ivec2 OtherUnitPos = GetBodyPos(&Sim->StaticBodyList, Node->ID);

              rvec2 PosDif = ConvertIvec2ToRvec2(CurrentUnitPos - OtherUnitPos);
              r32 SquaredDistance = CalcRvec2SquaredMagnitude(PosDif);
              if(SquaredDistance < SquaredTreeUnitDistanceMin) {
                r32 Distance = SquareRoot(SquaredDistance);
                rvec2 Direction = PosDif / Distance;
                r32 Overlap = SIMULATION_UNIT_HALF_SIZE + SIMULATION_TREE_HALF_SIZE - Distance;
                ivec2 Bounce = ConvertRvec2ToIvec2(Direction * Overlap * 1.001);
                SetBodyPosition(&Sim->DynamicBodyList, CurrentUnit->BodyID, CurrentUnitPos + Bounce);
                goto Retry;
              }
            }
          }
        }
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

ivec2 SimulationGetUnitPos(simulation *Sim, simulation_unit *Unit) {
  return GetBodyPos(&Sim->DynamicBodyList, Unit->BodyID);
}
