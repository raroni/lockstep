#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include "../shared.h"
#include "shared.h"

#include <stdio.h>

static int WakeReadFD;
static int WakeWriteFD;

void TestNetworkCommand() {
  ui8 X = 1;
  write(WakeWriteFD, &X, 1);
}

void InitNetwork2() {
  int FDs[2];
  pipe(FDs);
  WakeReadFD = FDs[0];
  WakeWriteFD = FDs[1];
}

void* RunNetwork(void *Data) {
  while(ServerRunning) {
    fd_set FDSet;
    FD_ZERO(&FDSet);
    // for(ui32 I=0; I<Network.ClientSet.Count; ++I) {
    //   FD_SET(Network.ClientSet.Clients[I].FD, &ClientFDSet);
    // }
    FD_SET(WakeReadFD, &FDSet);

    // todo: Max proper +1 here
    int SelectResult = select(WakeReadFD+1, &FDSet, NULL, NULL, NULL);
    Assert(SelectResult != -1);

    if(FD_ISSET(WakeReadFD, &FDSet)) {
      ui8 X;
      int Result = read(WakeReadFD, &X, 1);
      Assert(Result != -1);
    }

    printf("Unblocked\n");
  }

  close(WakeReadFD);
  close(WakeWriteFD);

  return NULL;
}
