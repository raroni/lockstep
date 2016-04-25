#include <stdio.h>
#include "lib/assert.h"
#include "common/memory.h"
#include "common/net_messages.h"
#include "common/simulation.h"
#include "net_events.h"
#include "net_commands.h"
#include "game.h"

#define MESSAGE_OUT_BUFFER_LENGTH 1024*10
static ui8 MessageOutBufferBlock[MESSAGE_OUT_BUFFER_LENGTH];
static buffer MessageOutBuffer = {
  .Addr = &MessageOutBufferBlock,
  .Length = sizeof(MessageOutBufferBlock)
};

struct player {
  simulation_player_id SimID;
  net_client_id ClientID;
};

#define PLAYERS_MAX 1
struct player_set {
  player Players[PLAYERS_MAX];
  memsize Count;
};

enum game_mode {
  game_mode_waiting_for_clients,
  game_mode_active,
  game_mode_disconnecting,
  game_mode_stopped
};

struct game_state {
  game_mode Mode;
  linear_allocator Allocator;
  player_set PlayerSet;
  uusec64 NextTickTime;
  simulation Sim;
};

static void InitPlayerSet(player_set *Set) {
  Set->Count = 0;
}

static bool FindPlayerByClientID(player_set *Set, net_client_id ID, memsize *Index) {
  for(memsize I=0; I<Set->Count; ++I) {
    if(Set->Players[I].ClientID == ID) {
      *Index = I;
      return true;
    }
  }
  return false;
}

static void AddPlayer(player_set *Set, simulation_player_id SimID, net_client_id NetID) {
  printf("Added player with client id %zu\n", NetID);
  player *Player = Set->Players + Set->Count;
  Player->ClientID = NetID;
  Player->SimID = SimID;
  Set->Count++;
}

static void Broadcast(const player_set *Set, const buffer Message, chunk_list *Commands) {
  net_client_id IDs[Set->Count];
  for(memsize I=0; I<Set->Count; ++I) {
    IDs[I] = Set->Players[I].ClientID;
  }

  static ui8 TempWorkBufferBlock[1024*1024];
  buffer TempWorkBuffer = {
    .Addr = TempWorkBufferBlock,
    .Length = sizeof(TempWorkBufferBlock)
  };

  memsize Length = SerializeBroadcastNetCommand(IDs, Set->Count, Message, TempWorkBuffer);
  buffer Command = {
    .Addr = TempWorkBuffer.Addr,
    .Length = Length
  };
  ChunkListWrite(Commands, Command);
}

static void RemovePlayer(player_set *Set, memsize Index) {
  Set->Count--;
}

void InitGame(buffer Memory) {
  game_state *State = (game_state*)Memory.Addr;

  {
    void *Base = (ui8*)Memory.Addr + sizeof(game_state);
    memsize Length = Memory.Length - sizeof(game_state);
    InitLinearAllocator(&State->Allocator, Base, Length);
  }

  InitPlayerSet(&State->PlayerSet);
}

void StartGame(game_state *State, chunk_list *NetCmds, uusec64 Time) {
  static ui8 TempWorkBufferBlock[1024*1024];
  buffer TempWorkBuffer = {
    .Addr = TempWorkBufferBlock,
    .Length = sizeof(TempWorkBufferBlock)
  };

  player_set *Set = &State->PlayerSet;
  for(memsize I=0; I<Set->Count; ++I) {
    memsize Length = SerializeStartNetMessage(Set->Count, I, MessageOutBuffer);
    buffer Message = {
      .Addr = MessageOutBuffer.Addr,
      .Length = Length
    };

    Length = SerializeSendNetCommand(Set->Players[I].ClientID, Message, TempWorkBuffer);
    buffer Command = {
      .Addr = TempWorkBuffer.Addr,
      .Length = Length
    };
    ChunkListWrite(NetCmds, Command);
  }


  State->NextTickTime = Time + SimulationTickDuration*1000;

  InitSimulation(&State->Sim);

  printf("Starting game...\n");
  State->Mode = game_mode_active;
}

void ProcessNetEvents(game_state *State, chunk_list *Events) {
  for(;;) {
    buffer Event = ChunkListRead(Events);
    if(Event.Length == 0) {
      break;
    }
    net_event_type Type = UnserializeNetEventType(Event);
    switch(Type) {
      case net_event_type_connect:
        printf("Game got connection event!\n");
        if(State->PlayerSet.Count != PLAYERS_MAX) {
          connect_net_event ConnectEvent = UnserializeConnectNetEvent(Event);
          simulation_player_id SimID = SimulationCreatePlayer(&State->Sim);
          AddPlayer(&State->PlayerSet, SimID, ConnectEvent.ClientID);
        }
        break;
      case net_event_type_disconnect: {
        printf("Game got disconnect event!\n");
        disconnect_net_event DisconnectEvent = UnserializeDisconnectNetEvent(Event);
        memsize PlayerIndex;
        bool Result = FindPlayerByClientID(&State->PlayerSet, DisconnectEvent.ClientID, &PlayerIndex);
        if(Result) {
          RemovePlayer(&State->PlayerSet, PlayerIndex);
          printf("Found and removed player with client ID %zu.\n", DisconnectEvent.ClientID);
        }
        break;
      }
      case net_event_type_reply: {
        reply_net_event ReplyEvent = UnserializeReplyNetEvent(Event);
        printf("Got reply from %zu.\n", ReplyEvent.ClientID);
        break;
      }
      default:
        InvalidCodePath;
    }
  }
}

void BroadcastLatestOrders(player_set *PlayerSet, chunk_list *Commands) {
  memsize Length = SerializeOrderListNetMessage(MessageOutBuffer);
  buffer Message = {
    .Addr = MessageOutBuffer.Addr,
    .Length = Length
  };

  Broadcast(PlayerSet, Message, Commands);
}

void UpdateGame(
  uusec64 Time,
  uusec64 *Delay,
  bool TerminationRequested,
  chunk_list *Events,
  chunk_list *Commands,
  bool *Running,
  buffer Memory
) {
  game_state *State = (game_state*)Memory.Addr;

  static ui8 TempWorkBufferBlock[1024*1024*5];
  buffer TempWorkBuffer = {
    .Addr = TempWorkBufferBlock,
    .Length = sizeof(TempWorkBufferBlock)
  };

  ProcessNetEvents(State, Events);

  if(State->Mode != game_mode_disconnecting && TerminationRequested) {
    State->Mode = game_mode_disconnecting;
    memsize Length = SerializeShutdownNetCommand(TempWorkBuffer);
    buffer Command = {
      .Addr = TempWorkBuffer.Addr,
      .Length = Length
    };
    ChunkListWrite(Commands, Command);
  }
  else if(State->Mode != game_mode_waiting_for_clients && State->PlayerSet.Count == 0) {
    printf("All players has left. Stopping game.\n");
    if(State->Mode != game_mode_disconnecting) {
      memsize Length = SerializeShutdownNetCommand(TempWorkBuffer);
      buffer Command = {
        .Addr = TempWorkBuffer.Addr,
        .Length = Length
      };
      ChunkListWrite(Commands, Command);
    }
    *Running = false;
    State->Mode = game_mode_stopped;
  }
  else if(State->Mode == game_mode_waiting_for_clients && State->PlayerSet.Count == PLAYERS_MAX) {
      StartGame(State, Commands, Time);
  }
  else if(State->Mode == game_mode_active) {
    if(Time >= State->NextTickTime) {
      BroadcastLatestOrders(&State->PlayerSet, Commands);
      State->NextTickTime += SimulationTickDuration*1000;

      simulation_order_list DummyOrderList;
      DummyOrderList.Count = 0;
      TickSimulation(&State->Sim, &DummyOrderList);
    }
  }
  else if(State->Mode == game_mode_disconnecting) {
    // TODO: If players doesn't perform clean disconnect
    // we should just continue after a timeout.
  }
  *Delay = 1000;
}
