#include "lib/assert.h"
#include "lib/math.h"
#include "simulation.h"

#define UNITS_PER_PLAYER 256

typedef simulation_unit unit;
typedef simulation_player_id player_id;
typedef simulation_order_list order_list;
typedef simulation_player player;
typedef simulation_body_cell_node body_cell_node;
typedef simulation_body_id body_id;
typedef simulation_body_list body_list;
typedef simulation_body_cell body_cell;

struct collision {
  bool Hit;
  body_id ColliderID;
  ivec2 ColliderPos;
  rvec2 Direction;
  r32 DistanceViolation;
};

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

static void InitBodyList(simulation_body_list *List, ui16 Max, memory_arena *Arena) {
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

collision FindCollision(
  body_list *List, ui16 CellIndex,
  body_id BaseBodyID, ivec2 BasePos,
  r32 DistanceMin, r32 DistanceViolationMin
) {
  static const r32 DistanceEpsilon = 0.001;

  r32 SquaredDistanceMin = DistanceMin * DistanceMin;

  collision Collision;
  Collision.Hit = false;
  body_cell *Cell = List->Cells + CellIndex;
  for(body_cell_node *Node = Cell->First; Node; Node = Node->Next) {
    if(Node->ID == BaseBodyID) {
      continue;
    }
    ivec2 ColliderPos = GetBodyPos(List, Node->ID);
    rvec2 PosDif = ConvertIvec2ToRvec2(BasePos - ColliderPos);
    r32 SquaredDistance = CalcRvec2SquaredMagnitude(PosDif);
    if(SquaredDistance < SquaredDistanceMin) {
      r32 Distance = SquareRoot(SquaredDistance);
      Collision.Direction = {};
      if(Distance < DistanceEpsilon) {
        Collision.Direction.X = 1;
      }
      else {
        Collision.Direction = PosDif / Distance;
      }
      Collision.DistanceViolation = MaxR32(DistanceMin - Distance, DistanceViolationMin);
      Collision.ColliderID = Node->ID;
      Collision.ColliderPos = ColliderPos;
      Collision.Hit = true;
      break;
    }
  }
  return Collision;
}

static void PerformCollisions(simulation *Sim) {
  r32 TreeUnitDistanceMin = SIMULATION_TREE_HALF_SIZE + SIMULATION_UNIT_HALF_SIZE;
  r32 UnitUnitDistanceMin = SIMULATION_UNIT_HALF_SIZE * 2;

  for(memsize U1=0; U1<Sim->UnitCount; ++U1) {
    unit *CurrentUnit = Sim->Units + U1;
    memsize RetryCount = 0;
    Retry:
    if(RetryCount++ < 4) {
      ivec2 CurrentUnitPos = GetBodyPos(&Sim->DynamicBodyList, CurrentUnit->BodyID);

      irect CellRect;
      {
        ivec2 SimPosMin;
        SimPosMin.X = CurrentUnitPos.X - SIMULATION_ENTITY_MAX_SIZE;
        SimPosMin.Y = CurrentUnitPos.Y - SIMULATION_ENTITY_MAX_SIZE;
        SimPosMin = ClampSimPos(SimPosMin);
        CellRect.Min = CalcCellPos(SimPosMin);

        ivec2 SimPosMax;
        SimPosMax.X = CurrentUnitPos.X + SIMULATION_ENTITY_MAX_SIZE;
        SimPosMax.Y = CurrentUnitPos.Y + SIMULATION_ENTITY_MAX_SIZE;
        SimPosMax = ClampSimPos(SimPosMax);
        CellRect.Max = CalcCellPos(SimPosMax);
      }

      for(memsize CellY=CellRect.Min.Y; CellY<=CellRect.Max.Y; ++CellY) {
        for(memsize CellX=CellRect.Min.X; CellX<=CellRect.Max.X; ++CellX) {
          ivec2 CellPos = MakeIvec2(CellX, CellY);
          ui16 CellIndex = CalcCellIndexByCellPos(CellPos);

          collision Collision = FindCollision(
            &Sim->DynamicBodyList, CellIndex, CurrentUnit->ID,
            CurrentUnitPos, UnitUnitDistanceMin, 2.0f
          );
          if(Collision.Hit) {
            ivec2 Bounce = ConvertRvec2ToIvec2(Collision.Direction * Collision.DistanceViolation * 0.501f);
            SetBodyPosition(&Sim->DynamicBodyList, CurrentUnit->BodyID, CurrentUnitPos + Bounce);
            SetBodyPosition(&Sim->DynamicBodyList, Collision.ColliderID, Collision.ColliderPos - Bounce);
            goto Retry;
          }

          Collision = FindCollision(
            &Sim->StaticBodyList, CellIndex, CurrentUnit->ID,
            CurrentUnitPos, TreeUnitDistanceMin, 1.0f
          );
          if(Collision.Hit) {
            ivec2 Bounce = ConvertRvec2ToIvec2(Collision.Direction * Collision.DistanceViolation * 1.001);
            SetBodyPosition(&Sim->DynamicBodyList, CurrentUnit->BodyID, CurrentUnitPos + Bounce);
            goto Retry;
          }
        }
      }
    }
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

memsize SimulationFindUnits(simulation *Sim, irect SimRect, simulation_unit_id *IDs, memsize Max) {
  irect CellRect;
  SimRect.Min = ClampSimPos(SimRect.Min);
  CellRect.Min = CalcCellPos(SimRect.Min);
  SimRect.Max = ClampSimPos(SimRect.Max);
  CellRect.Max = CalcCellPos(SimRect.Max);

  memsize Count = 0;
  for(memsize CellY=CellRect.Min.Y; CellY<=CellRect.Max.Y; ++CellY) {
    for(memsize CellX=CellRect.Min.X; CellX<=CellRect.Max.X; ++CellX) {
      ivec2 CellPos = MakeIvec2(CellX, CellY);
      ui16 CellIndex = CalcCellIndexByCellPos(CellPos);

      body_cell *Cell = Sim->DynamicBodyList.Cells + CellIndex;
      for(body_cell_node *Node = Cell->First; Node; Node = Node->Next) {
        ivec2 UnitPos = GetBodyPos(&Sim->DynamicBodyList, Node->ID);
        if(InsideIrect(SimRect, UnitPos)) {
          IDs[Count++] = Node->ID;
          if(Count == Max) {
            return Count;
          }
        }
      }
    }
  }

  return Count;
}

simulation_player_id SimulationCreatePlayer(simulation *Sim) {
  Assert(Sim->PlayerCount != SIMULATION_PLAYER_MAX);
  player *Player = Sim->Players + Sim->PlayerCount;
  Player->ID = Sim->PlayerCount;
  memsize StartPositionCount = sizeof(StartPositions) / sizeof(StartPositions[0]);
  ivec2 Base = StartPositions[Player->ID % StartPositionCount];

  memsize UnitCount = 0;
  memsize RowColCount = Ceil(SquareRoot((r32)UNITS_PER_PLAYER));
  memsize Spacing = 30;
  memsize GridSize = Spacing * (RowColCount - 1);
  for(memsize X=0; X<RowColCount; ++X) {
    for(memsize Y=0; Y<RowColCount && UNITS_PER_PLAYER > UnitCount; ++Y) {
      ivec2 Translation;
      Translation.X = ((r32)X / RowColCount - 0.5) * GridSize;
      Translation.Y = ((r32)Y / RowColCount - 0.5) * GridSize;
      ivec2 Pos = Base + Translation;
      CreateUnit(Sim, Player->ID, Pos);
      UnitCount++;
    }
  }
  Sim->PlayerCount++;
  return Player->ID;
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
