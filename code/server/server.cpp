#include <stdio.h>
#include "lib/assert.h"
#include "common/memory.h"
#include "common/network_messages.h"
#include "network_events.h"
#include "network_commands.h"
#include "network.h"
#include "server.h"

#define MESSAGE_OUT_BUFFER_LENGTH 1024*10
static ui8 MessageOutBufferBlock[MESSAGE_OUT_BUFFER_LENGTH];
static buffer MessageOutBuffer = {
  .Addr = &MessageOutBufferBlock,
  .Length = sizeof(MessageOutBufferBlock)
};

struct player {
  client_id ClientID;
};

#define PLAYERS_MAX 1
struct player_set {
  player Players[PLAYERS_MAX];
  memsize Count;
};

enum server_mode {
  server_mode_waiting_for_clients,
  server_mode_active,
  server_mode_disconnecting,
  server_mode_stopped
};

struct server_state {
  server_mode Mode;
  linear_allocator Allocator;
  player_set PlayerSet;
};

static void InitPlayerSet(player_set *Set) {
  Set->Count = 0;
}

static bool FindPlayerByClientID(player_set *Set, client_id ID, memsize *Index) {
  for(memsize I=0; I<Set->Count; ++I) {
    if(Set->Players[I].ClientID == ID) {
      *Index = I;
      return true;
    }
  }
  return false;
}

static void AddPlayer(player_set *Set, client_id ID) {
  printf("Added player with client id %zu\n", ID);
  Set->Players[Set->Count++].ClientID = ID;
}

static void Broadcast(const player_set *Set, const buffer Message, chunk_list *Commands) {
  printf("Request broadcast!\n");
  client_id IDs[Set->Count];
  for(memsize I=0; I<Set->Count; ++I) {
    IDs[I] = Set->Players[I].ClientID;
  }

  static ui8 TempWorkBufferBlock[1024*1024];
  buffer TempWorkBuffer = {
    .Addr = TempWorkBufferBlock,
    .Length = sizeof(TempWorkBufferBlock)
  };

  memsize Length = SerializeBroadcastNetworkCommand(IDs, Set->Count, Message, TempWorkBuffer);
  buffer Command = {
    .Addr = TempWorkBuffer.Addr,
    .Length = Length
  };
  ChunkListWrite(Commands, Command);
}

static void RemovePlayer(player_set *Set, memsize Index) {
  Set->Count--;
}

void InitServer(buffer Memory) {
  server_state *State = (server_state*)Memory.Addr;

  {
    void *Base = (ui8*)Memory.Addr + sizeof(server_state);
    memsize Length = Memory.Length - sizeof(server_state);
    InitLinearAllocator(&State->Allocator, Base, Length);
  }

  InitPlayerSet(&State->PlayerSet);
}

void UpdateServer(
  bool TerminationRequested,
  chunk_list *Events,
  chunk_list *Commands,
  bool *Running,
  buffer Memory
) {
  server_state *State = (server_state*)Memory.Addr;

  static ui8 TempWorkBufferBlock[1024*1024*5];
  buffer TempWorkBuffer = {
    .Addr = TempWorkBufferBlock,
    .Length = sizeof(TempWorkBufferBlock)
  };

  for(;;) {
    buffer Event = ChunkListRead(Events);
    if(Event.Length == 0) {
      break;
    }
    network_event_type Type = UnserializeNetworkEventType(Event);
    switch(Type) {
      case network_event_type_connect:
        printf("Game got connection event!\n");
        if(State->PlayerSet.Count != PLAYERS_MAX) {
          connect_network_event ConnectEvent = UnserializeConnectNetworkEvent(Event);
          AddPlayer(&State->PlayerSet, ConnectEvent.ClientID);
        }
        break;
      case network_event_type_disconnect: {
        printf("Game got disconnect event!\n");
        disconnect_network_event DisconnectEvent = UnserializeDisconnectNetworkEvent(Event);
        memsize PlayerIndex;
        bool Result = FindPlayerByClientID(&State->PlayerSet, DisconnectEvent.ClientID, &PlayerIndex);
        if(Result) {
          RemovePlayer(&State->PlayerSet, PlayerIndex);
          printf("Found and removed player with client ID %zu.\n", DisconnectEvent.ClientID);
        }
        break;
      }
      case network_event_type_reply: {
        reply_network_event ReplyEvent = UnserializeReplyNetworkEvent(Event);
        printf("Got reply from %zu.\n", ReplyEvent.ClientID);
        break;
      }
      default:
        InvalidCodePath;
    }
  }

  if(State->Mode != server_mode_disconnecting && TerminationRequested) {
    State->Mode = server_mode_disconnecting;
    memsize Length = SerializeShutdownNetworkCommand(TempWorkBuffer);
    buffer Command = {
      .Addr = TempWorkBuffer.Addr,
      .Length = Length
    };
    ChunkListWrite(Commands, Command);
  }
  else if(State->Mode != server_mode_waiting_for_clients && State->PlayerSet.Count == 0) {
    printf("All players has left. Stopping game.\n");
    if(State->Mode != server_mode_disconnecting) {
      memsize Length = SerializeShutdownNetworkCommand(TempWorkBuffer);
      buffer Command = {
        .Addr = TempWorkBuffer.Addr,
        .Length = Length
      };
      ChunkListWrite(Commands, Command);
    }
    *Running = false;
    State->Mode = server_mode_stopped;
  }
  else {
    if(State->Mode == server_mode_waiting_for_clients && State->PlayerSet.Count == PLAYERS_MAX) {
      memsize Length = SerializeStartNetworkMessage(MessageOutBuffer);
      buffer MessageBuffer = {
        .Addr = MessageOutBuffer.Addr,
        .Length = Length
      };
      Broadcast(&State->PlayerSet, MessageBuffer, Commands);
      printf("Starting game...\n");
      State->Mode = server_mode_active;
    }
    else if(State->Mode == server_mode_active) {
    }
    else if(State->Mode == server_mode_disconnecting) {
      // TODO: If players doesn't perform clean disconnect
      // we should just continue after a timeout.
    }
  }
}
