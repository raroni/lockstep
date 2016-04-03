#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include "shared.h"
#include "network.h"
#include "../shared.h"
#include "server_network.h"
#include "../packet.h"

enum game_state {
  game_state_waiting_for_clients,
  game_state_active,
  game_state_disconnecting
};

static bool DisconnectRequested;

struct main_state {
  game_state GameState;
  pthread_t NetworkThread;
};

static void HandleSignal(int signum) {
  DisconnectRequested = true;
}

enum packet_type {
  packet_type_start
};

static packet Packet;
#define PACKET_BUFFER_SIZE 1024*10
static ui8 PacketBuffer[PACKET_BUFFER_SIZE];

int main() {
  ServerRunning = true;

  main_state MainState;
  MainState.GameState = game_state_waiting_for_clients;

  InitNetwork2();
  pthread_create(&MainState.NetworkThread, 0, RunNetwork, 0);

  DisconnectRequested = false;
  int TargetClientCount = 1;
  ResetPacket(&Packet);
  Packet.Data = &PacketBuffer;
  Packet.Capacity = PACKET_BUFFER_SIZE;

  // InitNetwork();
  printf("Listening...\n");

  signal(SIGINT, HandleSignal);

  while(ServerRunning) {
    // UpdateNetwork();

    if(MainState.GameState != game_state_disconnecting && DisconnectRequested) {
      MainState.GameState = game_state_disconnecting;
      DisconnectNetwork2();
    }
    else if(MainState.GameState != game_state_waiting_for_clients && Network.ClientSet.Count == 0) {
      printf("All players has left. Stopping game.\n");
      // TestNetworkCommand();
      ServerRunning = false;
    }
    else {
      if(MainState.GameState == game_state_waiting_for_clients && Network.ClientSet.Count == TargetClientCount) {
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

  TerminateNetwork2();
  printf("Gracefully terminated.\n");
  return 0;
}
