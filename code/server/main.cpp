#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include "../shared.h"
#include "server_network.h"
#include "../packet.h"

enum game_state {
  game_state_waiting_for_clients,
  game_state_active,
  game_state_disconnecting
};

static game_state GameState;
static bool Running;
static bool DisconnectRequested;

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
  DisconnectRequested = false;
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

    if(GameState != game_state_disconnecting && DisconnectRequested) {
      GameState = game_state_disconnecting;
      DisconnectNetwork();
    }
    else if(GameState != game_state_waiting_for_clients && Network.ClientSet.Count == 0) {
      printf("All players has left. Stopping game.\n");
      Running = false;
    }
    else {
      if(GameState == game_state_waiting_for_clients && Network.ClientSet.Count == TargetClientCount) {
        ui8 TypeInt = SafeCastIntToUI8(packet_type_start);
        PacketWriteUI8(&Packet, TypeInt);
        NetworkBroadcast(Packet.Data, Packet.Length);
        printf("Starting game...\n");
        GameState = game_state_active;
      }
      else if(GameState == game_state_active) {
      }
      else if(GameState == game_state_disconnecting) {
        // TODO: If players doesn't perform clean disconnect
        // we should just continue after a timeout.
      }
    }
  }

  TerminateNetwork();
  printf("Gracefully terminated.\n");
  return 0;
}
