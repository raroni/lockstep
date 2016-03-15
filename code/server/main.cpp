#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include "../shared.h"
#include "server_network.h"

enum game_state {
  game_state_waiting_for_clients,
  game_state_active
};

static game_state GameState;
static bool Running;

static void HandleSignal(int signum) {
  Running = false;
}

int main() {
  int TargetClientCount = 1;

  InitNetwork();
  printf("Listening...\n");

  signal(SIGINT, HandleSignal);

  GameState = game_state_waiting_for_clients;

  Running = true;
  while(Running) {
    UpdateNetwork();

    if(GameState == game_state_waiting_for_clients && Network.ClientSet.Count == TargetClientCount) {
      // NetworkBroadcast(Start);
      printf("Starting game...\n");
      GameState = game_state_active;
    }
    else if(GameState == game_state_active) {
      if(Network.ClientSet.Count == 0) {
        printf("All players left. Stopping game.\n");
        Running = false;
        break;
      }
    }
  }

  TerminateNetwork();
  printf("Gracefully terminated.\n");
  return 0;
}
