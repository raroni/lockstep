#include <stdio.h>
#include "lib/assert.h"
#include "common/memory.h"
#include "common/net_messages.h"
#include "common/simulation.h"
#include "net_events.h"
#include "common/order_serialization.h"
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
  chunk_list OrderQueue;
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

static simulation_player_id FindSimIDByClientID(player_set *Set, net_client_id ClientID) {
  for(memsize I=0; I<Set->Count; ++I) {
    if(Set->Players[I].ClientID == ClientID) {
      return Set->Players[I].SimID;
    }
  }
  return SIMULATION_UNDEFINED_PLAYER_ID;
}

static void AddPlayer(player_set *Set, net_client_id NetID) {
  printf("Added player with client id %zu\n", NetID);
  player *Player = Set->Players + Set->Count;
  Player->ClientID = NetID;
  Player->SimID = 0;
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

  {
    memsize Length = 1024*20;
    void *Storage = LinearAllocate(&State->Allocator, Length);
    buffer Buffer = {
      .Addr = Storage,
      .Length = Length
    };
    InitChunkList(&State->OrderQueue, Buffer);
  }

  InitPlayerSet(&State->PlayerSet);
}

void StartGame(game_state *State, chunk_list *NetCmds, uusec64 Time) {
  player_set *Set = &State->PlayerSet;

  InitSimulation(&State->Sim);
  for(memsize I=0; I<Set->Count; ++I) {
    Set->Players[I].SimID = SimulationCreatePlayer(&State->Sim);
  }

  static ui8 TempWorkBufferBlock[1024*1024];
  buffer TempWorkBuffer = {
    .Addr = TempWorkBufferBlock,
    .Length = sizeof(TempWorkBufferBlock)
  };

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

  printf("Starting game...\n");
  State->Mode = game_mode_active;
}

void ProcessMessageEvent(message_net_event Event, player_set *PlayerSet, linear_allocator *Allocator, chunk_list *OrderQueue) {
  net_message_type Type = UnserializeNetMessageType(Event.Message);
  switch(Type) {
    case net_message_type_reply:
      printf("Received reply.\n");
      break;
    case net_message_type_order: {
      linear_allocator_context LAContext = CreateLinearAllocatorContext(Allocator);
      order_net_message Message = UnserializeOrderNetMessage(Event.Message, Allocator);

      simulation_order Order;
      Order.PlayerID = FindSimIDByClientID(PlayerSet, Event.ClientID);
      if(Order.PlayerID != SIMULATION_UNDEFINED_PLAYER_ID) {
        Order.UnitIDs = Message.UnitIDs;
        Order.UnitCount = Message.UnitCount;
        Order.Target = Message.Target;

        buffer OrderBuffer = SerializeOrder(Order, Allocator);
        ChunkListWrite(OrderQueue, OrderBuffer);
      }

      RestoreLinearAllocatorContext(LAContext);
      break;
    }
    default:
      InvalidCodePath;
  }
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
          AddPlayer(&State->PlayerSet, ConnectEvent.ClientID);
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
      case net_event_type_message: {
        message_net_event MessageEvent = UnserializeMessageNetEvent(Event);
        printf("Got message from client %zu of length %zu\n", MessageEvent.ClientID, MessageEvent.Message.Length);
        ProcessMessageEvent(MessageEvent, &State->PlayerSet, &State->Allocator, &State->OrderQueue);
        break;
      }
      default:
        InvalidCodePath;
    }
  }
}

void BroadcastOrders(player_set *PlayerSet, simulation_order_list *SimOrderList, chunk_list *Commands, linear_allocator *Allocator) {
  linear_allocator_context LAContext = CreateLinearAllocatorContext(Allocator);

  net_message_order *NetOrders = NULL;
  if(SimOrderList->Count != 0) {
    memsize NetOrdersSize = sizeof(net_message_order) * SimOrderList->Count;
    NetOrders = (net_message_order*)LinearAllocate(Allocator, NetOrdersSize);
    for(memsize I=0; I<SimOrderList->Count; ++I) {
      net_message_order *NetOrder = NetOrders + I;
      simulation_order *SimOrder = SimOrderList->Orders + I;
      NetOrder->PlayerID = SimOrder->PlayerID;
      NetOrder->UnitCount = SimOrder->UnitCount;
      NetOrder->Target = SimOrder->Target;

      memsize NetOrderUnitIDsSize = sizeof(ui16) * NetOrder->UnitCount;
      NetOrder->UnitIDs = (ui16*)LinearAllocate(Allocator, NetOrderUnitIDsSize);
      for(memsize U=0; U<NetOrder->UnitCount; ++U) {
        NetOrder->UnitIDs[U] = SimOrder->UnitIDs[U];
      }
    }
  }

  memsize Length = SerializeOrderListNetMessage(NetOrders, SimOrderList->Count, MessageOutBuffer);
  buffer Message = {
    .Addr = MessageOutBuffer.Addr,
    .Length = Length
  };
  Broadcast(PlayerSet, Message, Commands);
  RestoreLinearAllocatorContext(LAContext);
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
      linear_allocator_context LAContext = CreateLinearAllocatorContext(&State->Allocator);

      simulation_order_list OrderList;
      OrderList.Count = State->OrderQueue.Count;
      OrderList.Orders = NULL;
      if(OrderList.Count != 0) {
        memsize OrderListSize = sizeof(simulation_order) * State->OrderQueue.Count;
        OrderList.Orders = (simulation_order*)LinearAllocate(&State->Allocator, OrderListSize);

        for(memsize I=0; I<State->OrderQueue.Count; ++I) {
          buffer OrderBuffer = ChunkListRead(&State->OrderQueue);
          OrderList.Orders[I] = UnserializeOrder(OrderBuffer, &State->Allocator);
        }
      }
      BroadcastOrders(&State->PlayerSet, &OrderList, Commands, &State->Allocator);
      TickSimulation(&State->Sim, &OrderList);
      ResetChunkList(&State->OrderQueue);
      RestoreLinearAllocatorContext(LAContext);
      State->NextTickTime += SimulationTickDuration*1000;
    }
  }
  else if(State->Mode == game_mode_disconnecting) {
    // TODO: If players doesn't perform clean disconnect
    // we should just continue after a timeout.
  }
  *Delay = 1000;
}
