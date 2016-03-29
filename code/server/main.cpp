#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include "../shared.h"
#include "server_network.h"
#include "../packet.h"

enum game_state {
  game_state_waiting_for_clients,
  game_state_active
};

static game_state GameState;
static bool Running;

static void HandleSignal(int signum) {
  Running = false;
}

enum packet_type {
  packet_type_start
};

static packet Packet;
#define PACKET_BUFFER_SIZE 1024*10
static ui8 PacketBuffer[PACKET_BUFFER_SIZE];

int main() {
  int TargetClientCount = 1;
  ResetPacket(&Packet);
  Packet.Data = &PacketBuffer;
  Packet.Capacity = PACKET_BUFFER_SIZE;

  InitNetwork();
  printf("Listening...\n");

  signal(SIGINT, HandleSignal);

  GameState = game_state_waiting_for_clients;

  Running = true;
  while(Running) {
    UpdateNetwork();

    if(GameState == game_state_waiting_for_clients && Network.ClientSet.Count == TargetClientCount) {
      ui8 TypeInt = SafeCastIntToUI8(packet_type_start);
      PacketWriteUI8(&Packet, TypeInt);
      NetworkBroadcast(Packet.Data, Packet.Length);
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
