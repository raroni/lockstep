#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "../shared.h"
#include "network_client.h"

static bool Running;

static void HandleSigint(int signum) {
  Running = false;
}

int main() {
  signal(SIGINT, HandleSigint);

  InitNetwork();

  Running = true;
  while(Running) {
    if(Network.State != network_state_inactive) {
      UpdateNetwork();
    }
    else {
      break;
    }
  }

  TerminateNetwork();
  printf("\nGracefully terminated.\n");
  return 0;
}
