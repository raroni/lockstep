#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include "../shared.h"
#include "server_network.h"

static bool Running;

static void HandleSignal(int signum) {
  Running = false;
}

int main() {
  InitNetwork();
  printf("Listening...\n");

  signal(SIGINT, HandleSignal);

  Running = true;
  while(Running) {
    UpdateNetwork();
  }
  TerminateNetwork();
  printf("\nGracefully terminated.");
  return 0;
}
