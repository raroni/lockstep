#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include "lib/assert.h"
#include "common/shared.h"
#include "common/packet.h"
#include "network.h"

enum game_state {
  game_state_waiting_for_clients,
  game_state_active,
  game_state_disconnecting,
  game_state_stopped
};

static bool DisconnectRequested;

struct player {

};

#define PLAYERS_MAX 1
struct player_set {
  player Players[PLAYERS_MAX];
  memsize Count;
};

struct main_state {
  game_state GameState;
  pthread_t NetworkThread;
  player_set PlayerSet;
};

static void HandleSignal(int signum) {
  DisconnectRequested = true;
}

enum packet_type {
  packet_type_start
};

void InitPlayerSet(player_set *Set) {
  Set->Count = 0;
}

void AddPlayer(player_set *Set) {
  Set->Count++;
}

void RemovePlayer(player_set *Set) {
  Set->Count--;
}

static packet Packet;
#define PACKET_BUFFER_SIZE 1024*10
static ui8 PacketBuffer[PACKET_BUFFER_SIZE];

int main() {
  main_state MainState;
  MainState.GameState = game_state_waiting_for_clients;

  InitNetwork();
  pthread_create(&MainState.NetworkThread, 0, RunNetwork, 0);

  InitPlayerSet(&MainState.PlayerSet);

  DisconnectRequested = false;
  ResetPacket(&Packet);
  Packet.Data = &PacketBuffer;
  Packet.Capacity = PACKET_BUFFER_SIZE;

  printf("Listening...\n");

  signal(SIGINT, HandleSignal);

  while(MainState.GameState != game_state_stopped) {
    {
      memsize Length;
      network_base_event *BaseEvent;
      while((Length = ReadNetworkEvent(&BaseEvent))) {
        switch(BaseEvent->Type) {
          case network_event_type_connect:
            printf("Game got connection event!\n");
            if(MainState.PlayerSet.Count != PLAYERS_MAX) {
              AddPlayer(&MainState.PlayerSet);
            }
            break;
          case network_event_type_disconnect:
            printf("Game got disconnect event!\n");
            RemovePlayer(&MainState.PlayerSet);
            break;
          default:
            InvalidCodePath;
        }
      }
    }

    if(MainState.GameState != game_state_disconnecting && DisconnectRequested) {
      MainState.GameState = game_state_disconnecting;
      DisconnectNetwork();
    }
    else if(MainState.GameState != game_state_waiting_for_clients && MainState.PlayerSet.Count == 0) {
      printf("All players has left. Stopping game.\n");
      if(MainState.GameState != game_state_disconnecting) {
        DisconnectNetwork();
      }
      MainState.GameState = game_state_stopped;
    }
    else {
      if(MainState.GameState == game_state_waiting_for_clients && MainState.PlayerSet.Count == PLAYERS_MAX) {
        ui8 TypeInt = SafeCastIntToUI8(packet_type_start);
        PacketWriteUI8(&Packet, TypeInt);
        // NetworkBroadcast(Packet.Data, Packet.Length);
        printf("Starting game...\n");
        MainState.GameState = game_state_active;
      }
      else if(MainState.GameState == game_state_active) {
      }
      else if(MainState.GameState == game_state_disconnecting) {
        // TODO: If players doesn't perform clean disconnect
        // we should just continue after a timeout.
      }
    }
  }

  pthread_join(MainState.NetworkThread, 0);

  TerminateNetwork();
  printf("Gracefully terminated.\n");
  return 0;
}
