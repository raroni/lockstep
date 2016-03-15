#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "../shared.h"
#include "network_client.h"

static bool Running;

static void HandleSigint(int signum) {
  Running = false;
}

#define TEST_BUFFER_SIZE 4096
ui8 TestBuffer[TEST_BUFFER_SIZE];

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
