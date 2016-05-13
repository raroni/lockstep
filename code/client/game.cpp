#include <stdio.h>
#include <math.h>
#include "lib/def.h"
#include "lib/assert.h"
#include "lib/int_seq.h"
#include "lib/chunk_ring_buffer.h"
#include "lib/memory_arena.h"
#include "common/net_messages.h"
#include "common/simulation.h"
#include "common/order_serialization.h"
#include "coors.h"
#include "interpolation.h"
#include "net_events.h"
#include "net_commands.h"
#include "render_commands.h"
#include "game.h"

static const ui32 DarkBlueColor = 0x002D3E50;
static const ui32 RedColor = 0x00E64E42;
static const ui32 BlueColor = 0x002F81B8;
static const ui32 PurpleColor = 0x008D48AB;
static const ui32 OrangeColor = 0x00E57F31;
static const ui32 WhiteColor = 0x00FFFFFF;
static const ui32 GreenColor = 0x0030AD63;
static const r32 Zoom = 1.0 / 1000.0;

static const ui32 PlayerColors[] = {
  RedColor, OrangeColor, BlueColor, PurpleColor
};

#define UNIT_SELECTION_MAX 128
struct unit_selection {
  simulation_unit_id IDs[UNIT_SELECTION_MAX];
  ui8 Count;
};

enum game_mode {
  game_mode_waiting,
  game_mode_active
};

struct drag_selection {
  bool Active;
  rrect Area;
};

struct game_state {
  memory_arena Arena;
  buffer CommandSerializationBuffer;
  drag_selection DragSelection;
  simulation_player_id PlayerID;
  simulation Sim;
  interpolation Interpolation;
  memsize *OrderListCounts;
  int_seq OrderListCountSeq;
  uusec64 NextTickTime;
  uusec64 NextExtraTickTime;
  chunk_ring_buffer OrderListRing;
  unit_selection UnitSelection;
  ivec2 LastMouseDownPos;
  bool MouseDragging;
  game_mode Mode;
};

static r32 CalcAspectRatio(ivec2 Resolution) {
  rvec2 Real = ConvertIvec2ToRvec2(Resolution);
  return Real.X / Real.Y;
}

void InitGame(buffer Memory) {
  game_state *State = (game_state*)Memory.Addr;
  {
    void *Base = (ui8*)Memory.Addr + sizeof(game_state);
    memsize Length = Memory.Length - sizeof(game_state);
    InitMemoryArena(&State->Arena, Base, Length);
  }

  {
    buffer *B = &State->CommandSerializationBuffer;
    B->Addr = MemoryArenaAllocate(&State->Arena, NET_COMMAND_MAX_LENGTH);
    B->Length = NET_COMMAND_MAX_LENGTH;
  }

  {
    usec32 WatchDuration = 10;
    memsize SamplesPerSecond = 1000 / SimulationTickDuration;
    memsize SequenceLength = SamplesPerSecond * WatchDuration;
    State->OrderListCounts = (memsize*)MemoryArenaAllocate(&State->Arena, sizeof(memsize)*SequenceLength);
    InitIntSeq(&State->OrderListCountSeq, State->OrderListCounts, SequenceLength);
  }

  {
    memsize Count = 100;
    memsize StorageSize = Count * 1024;
    void *Storage = MemoryArenaAllocate(&State->Arena, StorageSize);
    buffer Buffer = {
      .Addr = Storage,
      .Length = StorageSize
    };
    InitChunkRingBuffer(&State->OrderListRing, Count, Buffer);
  }

  State->PlayerID = SIMULATION_UNDEFINED_PLAYER_ID;
  State->UnitSelection.Count = 0;
  State->Mode = game_mode_waiting;
  State->MouseDragging = false;
  State->DragSelection.Active = false;
}

#define AddRenderCommand(List, Type) (Type##_render_command*)_AddRenderCommand(List, render_command_type_##Type, sizeof(Type##_render_command))

void* _AddRenderCommand(chunk_list *List, render_command_type Type, memsize Length) {
  Length += sizeof(Type);
  void *Base = ChunkListAllocate(List, Length);
  render_command_type *TypePtr = (render_command_type*)Base;
  *TypePtr = Type;
  ui8* Command = (ui8*)Base + sizeof(Type);
  return (void*)Command;
}

void Render(game_state *State, chunk_list *Commands, ivec2 Resolution) {
  {
    clear_color_render_command *Command = AddRenderCommand(Commands, clear_color);
    Command->Color = DarkBlueColor;
  }

  {
    projection_render_command *Command = AddRenderCommand(Commands, projection);
    Command->AspectRatio = CalcAspectRatio(Resolution);
    Command->Zoom = Zoom;
  }

  for(memsize I=0; I<State->UnitSelection.Count; ++I) {
    simulation_unit_id UnitID = State->UnitSelection.IDs[I];
    // TODO: Proper unit find based on ID (instead of index)
    // simulation_unit *Unit = GetSimulationUnit(UnitID);
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = State->Interpolation.Pos[UnitID].X;
    Command->Y = State->Interpolation.Pos[UnitID].Y;
    Command->Color = WhiteColor;
    Command->HalfSize = SIMULATION_UNIT_HALF_SIZE + 10;
  }

  for(memsize I=0; I<State->Sim.UnitCount; ++I) {
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = State->Interpolation.Pos[I].X;
    Command->Y = State->Interpolation.Pos[I].Y;
    Command->Color = PlayerColors[State->Sim.Units[I].PlayerID];
    Command->HalfSize = SIMULATION_UNIT_HALF_SIZE;
  }

  simulation_body_list *TreeList = &State->Sim.StaticBodyList;
  for(memsize I=0; I<TreeList->Count; ++I) {
    draw_square_render_command *Command = AddRenderCommand(Commands, draw_square);
    Command->X = TreeList->Poss[I].X;
    Command->Y = TreeList->Poss[I].Y;
    Command->Color = GreenColor;
    Command->HalfSize = SIMULATION_TREE_HALF_SIZE;
  }

  if(State->DragSelection.Active) {
    const static r32 Width = .01f;
    const static ui32 Color = OrangeColor;
    {
      projection_render_command *Command = AddRenderCommand(Commands, projection);
      Command->AspectRatio = CalcAspectRatio(Resolution);
      Command->Zoom = 1.0f;
    }

    {
      draw_rect_render_command *Command = AddRenderCommand(Commands, draw_rect);
      Command->Rect.Min = State->DragSelection.Area.Min;
      Command->Rect.Max.X = State->DragSelection.Area.Max.X;
      Command->Rect.Max.Y = State->DragSelection.Area.Min.Y + Width;
      Command->Color = Color;
    }
    {
      draw_rect_render_command *Command = AddRenderCommand(Commands, draw_rect);
      Command->Rect.Min.X = State->DragSelection.Area.Min.X;
      Command->Rect.Min.Y = State->DragSelection.Area.Max.Y - Width;
      Command->Rect.Max = State->DragSelection.Area.Max;
      Command->Color = Color;
    }
    {
      draw_rect_render_command *Command = AddRenderCommand(Commands, draw_rect);
      Command->Rect.Min = State->DragSelection.Area.Min;
      Command->Rect.Max.X = State->DragSelection.Area.Min.X + Width;
      Command->Rect.Max.Y = State->DragSelection.Area.Max.Y;
      Command->Color = Color;
    }
    {
      draw_rect_render_command *Command = AddRenderCommand(Commands, draw_rect);
      Command->Rect.Min.X = State->DragSelection.Area.Max.X - Width;
      Command->Rect.Min.Y = State->DragSelection.Area.Min.Y;
      Command->Rect.Max = State->DragSelection.Area.Max;
      Command->Color = Color;
    }
  }
}

void ProcessMessageEvent(message_net_event Event, game_state *State, chunk_list *NetCmds, uusec64 Time) {
  net_message_type MessageType = UnserializeNetMessageType(Event.Message);

  switch(MessageType) {
    case net_message_type_start: {
      start_net_message StartMessage = UnserializeStartNetMessage(Event.Message);
      printf("Game got start event. PlayerCount: %zu, PlayerID: %zu\n", StartMessage.PlayerCount, StartMessage.PlayerIndex);

      InitSimulation(&State->Sim, &State->Arena);
      for(memsize I=0; I<StartMessage.PlayerCount; ++I) {
        simulation_player_id PlayerID = SimulationCreatePlayer(&State->Sim);
        if(I == StartMessage.PlayerIndex) {
          State->PlayerID = PlayerID;
        }
      }
      Assert(State->PlayerID != SIMULATION_UNDEFINED_PLAYER_ID);
      InitInterpolation(&State->Interpolation, &State->Sim);

      memory_arena_checkpoint ArenaCheckpoint = CreateMemoryArenaCheckpoint(&State->Arena);
      Assert(GetMemoryArenaFree(&State->Arena) >= NET_MESSAGE_MAX_LENGTH + NET_COMMAND_MAX_LENGTH);

      buffer ReplyMessage = SerializeReplyNetMessage(&State->Arena);
      printf("Starting game and replying...\n");

      buffer Command = SerializeSendNetCommand(ReplyMessage, &State->Arena);
      ChunkListWrite(NetCmds, Command);
      ReleaseMemoryArenaCheckpoint(ArenaCheckpoint);

      State->NextTickTime = Time + SimulationTickDuration*1000;
      State->NextExtraTickTime = Time + SimulationTickDuration*1000;
      State->Mode = game_mode_active;
      break;
    }
    case net_message_type_order_list: {
      memory_arena_checkpoint ArenaCheckpoint = CreateMemoryArenaCheckpoint(&State->Arena);
      order_list_net_message ListMessage = UnserializeOrderListNetMessage(Event.Message, &State->Arena);

      simulation_order_list SimOrderList;
      SimOrderList.Count = ListMessage.Count;
      SimOrderList.Orders = NULL;
      if(SimOrderList.Count != 0) {
        memsize SimOrdersSize = sizeof(simulation_order) * SimOrderList.Count;
        SimOrderList.Orders = (simulation_order*)MemoryArenaAllocate(&State->Arena, SimOrdersSize);
        for(memsize I=0; I<SimOrderList.Count; ++I) {
          simulation_order *SimOrder = SimOrderList.Orders + I;
          net_message_order *NetOrder = ListMessage.Orders + I;
          SimOrder->PlayerID = NetOrder->PlayerID;
          SimOrder->UnitCount = NetOrder->UnitCount;
          SimOrder->Target = NetOrder->Target;

          memsize SimOrderUnitIDsSize = sizeof(simulation_unit_id) * SimOrder->UnitCount;
          SimOrder->UnitIDs = (simulation_unit_id*)MemoryArenaAllocate(&State->Arena, SimOrderUnitIDsSize);
          for(memsize U=0; U<SimOrder->UnitCount; ++U) {
            SimOrder->UnitIDs[U] = NetOrder->UnitIDs[U];
          }
        }
      }

      buffer SimOrderListBuffer = SerializeOrderList(&SimOrderList, &State->Arena);
      ChunkRingBufferWrite(&State->OrderListRing, SimOrderListBuffer);

      ReleaseMemoryArenaCheckpoint(ArenaCheckpoint);
      break;
    }
    default:
      InvalidCodePath;
  }
}

simulation_unit* FindUnit(simulation *Sim, ivec2 WorldPos) {
  for(memsize I=0; I<Sim->UnitCount; ++I) {
    simulation_unit *Unit = Sim->Units + I;
    ivec2 Pos = SimulationGetUnitPos(Sim, Unit);
    if(
      Pos.X - SIMULATION_UNIT_HALF_SIZE <= WorldPos.X &&
      Pos.X + SIMULATION_UNIT_HALF_SIZE > WorldPos.X &&
      Pos.Y - SIMULATION_UNIT_HALF_SIZE <= WorldPos.Y &&
      Pos.Y + SIMULATION_UNIT_HALF_SIZE > WorldPos.Y
    ) {
      return Unit;
    }
  }
  return NULL;
}

static void UnitSelectionEmpty(unit_selection *S) {
  S->Count = 0;
}

static void UnitSelectionAdd(unit_selection *S, simulation_unit_id ID) {
  Assert(S->Count != UNIT_SELECTION_MAX);
  S->IDs[S->Count++] = ID;
}

static void ProcessClick(game_state *State, game_mouse *Mouse, ivec2 Resolution, chunk_list *NetCmds)  {
  r32 AspectRatio = CalcAspectRatio(Resolution);
  ivec2 WorldPos = ConvertWindowToWorldCoors(Mouse->Pos, Resolution, AspectRatio, Zoom);
  simulation_unit *Unit = FindUnit(&State->Sim, WorldPos);

  if(Unit != NULL) {
    bool PerformSelect = State->UnitSelection.Count != 1 || State->UnitSelection.IDs[0] != Unit->ID;
    UnitSelectionEmpty(&State->UnitSelection);
    if(PerformSelect) {
      UnitSelectionAdd(&State->UnitSelection, Unit->ID);
    }
  }
  else if(State->UnitSelection.Count != 0) {
    memory_arena_checkpoint ArenaCheckpoint = CreateMemoryArenaCheckpoint(&State->Arena);
    Assert(GetMemoryArenaFree(&State->Arena) >= NET_MESSAGE_MAX_LENGTH + NET_COMMAND_MAX_LENGTH);

    buffer OrderMessage = SerializeOrderNetMessage(
      State->UnitSelection.IDs,
      State->UnitSelection.Count,
      WorldPos,
      &State->Arena
    );
    buffer Command = SerializeSendNetCommand(OrderMessage, &State->Arena);
    ChunkListWrite(NetCmds, Command);
    ReleaseMemoryArenaCheckpoint(ArenaCheckpoint);
  }
}

static void UpdateDragSelectionArea(game_state *State, game_mouse *Mouse, ivec2 Resolution) {
  r32 AspectRatio = CalcAspectRatio(Resolution);
  State->DragSelection.Area = CreateRrect(
    ConvertWindowToUICoors(Mouse->Pos, Resolution, AspectRatio),
    ConvertWindowToUICoors(State->LastMouseDownPos, Resolution, AspectRatio)
  );
}

static void ProcessDragStart(game_state *State, game_mouse *Mouse, ivec2 Resolution)  {
  State->DragSelection.Active = true;
  UpdateDragSelectionArea(State, Mouse, Resolution);
}

static void ProcessDragMove(game_state *State, game_mouse *Mouse, ivec2 Resolution)  {
  UpdateDragSelectionArea(State, Mouse, Resolution);
}

static void ProcessDragStop(game_state *State, game_mouse *Mouse, ivec2 Resolution)  {
  State->DragSelection.Active = false;

  r32 AspectRatio = CalcAspectRatio(Resolution);
  irect WorldRect;
  WorldRect.Min = ConvertUIToWorldCoors(State->DragSelection.Area.Min, AspectRatio, Zoom);
  WorldRect.Max = ConvertUIToWorldCoors(State->DragSelection.Area.Max, AspectRatio, Zoom);
  UnitSelectionEmpty(&State->UnitSelection);

  memory_arena_checkpoint ArenaCheckpoint = CreateMemoryArenaCheckpoint(&State->Arena);
  simulation_unit_id *UnitIDs;
  {
    memsize IDSize = sizeof(simulation_unit_id) * UNIT_SELECTION_MAX;
    UnitIDs = (simulation_unit_id*)MemoryArenaAllocate(&State->Arena, IDSize);
  }
  memsize Count = SimulationFindUnits(&State->Sim, WorldRect, UnitIDs, UNIT_SELECTION_MAX);
  for(memsize I=0; I<Count; ++I) {
    UnitSelectionAdd(&State->UnitSelection, UnitIDs[I]);
  }

  ReleaseMemoryArenaCheckpoint(ArenaCheckpoint);
}

static void ProcessMouse(game_state *State, game_mouse *Mouse, ivec2 Resolution, chunk_list *NetCmds) {
  if(Mouse->ButtonChangeCount != 0) {
    if(Mouse->ButtonPressed) {
      State->LastMouseDownPos = Mouse->Pos;
    }
    else {
      if(State->MouseDragging) {
        ProcessDragStop(State, Mouse, Resolution);
      }
      else {
        ProcessClick(State, Mouse, Resolution, NetCmds);
      }

      State->MouseDragging = false;
    }
  }

  if(Mouse->ButtonPressed) {
    if(!State->MouseDragging) {
      rvec2 Difference = ConvertIvec2ToRvec2(Mouse->Pos - State->LastMouseDownPos);
      r32 Distance = CalcRvec2Magnitude(Difference);
      if(Distance > 18.0f) {
        ProcessDragStart(State, Mouse, Resolution);
        State->MouseDragging = true;
      }
    }
    else {
      ProcessDragMove(State, Mouse, Resolution);
    }
  }
}

void RunSimulationTick(simulation *Sim, chunk_ring_buffer *OrderListRing, memory_arena *Arena) {
  Assert(GetChunkRingBufferUnreadCount(OrderListRing) != 0);
  buffer OrderListBuffer = ChunkRingBufferRefRead(OrderListRing);
  simulation_order_list OrderList = UnserializeOrderList(OrderListBuffer, Arena);
  TickSimulation(Sim, &OrderList);
}

void UpdateGame(game_platform *Platform, chunk_list *NetEvents, chunk_list *NetCmds, chunk_list *RenderCmds, bool *Running, buffer Memory) {
  game_state *State = (game_state*)Memory.Addr;

  if(State->Mode == game_mode_active) {
    ProcessMouse(State, Platform->Mouse, Platform->Resolution, NetCmds);
  }

  for(;;) {
    buffer Event = ChunkListRead(NetEvents);
    if(Event.Length == 0) {
      break;
    }
    net_event_type Type = UnserializeNetEventType(Event);
    switch(Type) {
      case net_event_type_connection_established:
        printf("Game got connection established!\n");
        break;
      case net_event_type_connection_lost:
        printf("Game got connection lost!\n");
        *Running = false;
        break;
      case net_event_type_connection_failed:
        printf("Game got connection failed!\n");
        *Running = false;
        break;
      case net_event_type_message: {
        message_net_event MessageEvent = UnserializeMessageNetEvent(Event);
        ProcessMessageEvent(MessageEvent, State, NetCmds, Platform->Time);
        break;
      }
      default:
        InvalidCodePath;
    }
  }

  if(State->Mode == game_mode_active) {
    if(Platform->Time >= State->NextTickTime) {
      memsize OrderListCount = GetChunkRingBufferUnreadCount(&State->OrderListRing);
      if(OrderListCount != 0) {
        RunSimulationTick(&State->Sim, &State->OrderListRing, &State->Arena);
        OrderListCount--;
        IntSeqPush(&State->OrderListCountSeq, OrderListCount);

        if(Platform->Time >= State->NextExtraTickTime && OrderListCount != 0) {
          double CountStdDev = CalcIntSeqStdDev(&State->OrderListCountSeq);
          static const umsec32 BaseFrameLag = 200;
          memsize TargetFrameLag = BaseFrameLag/SimulationTickDuration + round(CountStdDev * 4);
          if(OrderListCount > TargetFrameLag) {
            RunSimulationTick(&State->Sim, &State->OrderListRing, &State->Arena);
            State->NextExtraTickTime += 1000*1000;
          }
        }

        ReloadInterpolation(&State->Interpolation, &State->Sim);

        State->NextTickTime += SimulationTickDuration*1000;
      }
    }

    r32 TickProgress;
    {
      uusec64 TickDuration = SimulationTickDuration*1000;
      uusec64 TimeUntilNextTick = State->NextTickTime - Platform->Time;
      uusec64 TimeSpentInCurrentTick = TickDuration - TimeUntilNextTick;
      TickProgress = (r32)TimeSpentInCurrentTick / (r32)TickDuration;
      TickProgress = MinR32(TickProgress, 1.0f);
    }
    UpdateInterpolation(&State->Interpolation, &State->Sim, TickProgress);
    Render(State, RenderCmds, Platform->Resolution);
  }

  if(Platform->TerminationRequested) {
    printf("Requesting net shutdown...\n");

    memory_arena_checkpoint ArenaCheckpoint = CreateMemoryArenaCheckpoint(&State->Arena);
    Assert(GetMemoryArenaFree(&State->Arena) >= NET_COMMAND_MAX_LENGTH);
    buffer Command = SerializeShutdownNetCommand(&State->Arena);
    ChunkListWrite(NetCmds, Command);
    ReleaseMemoryArenaCheckpoint(ArenaCheckpoint);

    *Running = false;
  }
}
