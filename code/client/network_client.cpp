#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "../shared.h"
#include "../assert.h"
#include "../network.h"
#include "network_client.h"

network Network;

static int FD;

network_buffer ReceiveBuffer;

void InitReceiveBuffer() {
  size_t Capacity = 1024*100;
  void *Data = malloc(Capacity);
  InitNetworkBuffer(&ReceiveBuffer, Data, Capacity);
}

void InitNetwork() {
  InitReceiveBuffer();

  FD = socket(PF_INET, SOCK_STREAM, 0);
  Assert(FD != -1);
  fcntl(FD, F_SETFL, O_NONBLOCK);

  struct sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_len = sizeof(Address);
  Address.sin_family = AF_INET;
  Address.sin_port = htons(4321);
  Address.sin_addr.s_addr = 0;

  int ConnectResult = connect(FD, (struct sockaddr *)&Address, sizeof(Address));
  Assert(ConnectResult != -1 || errno == errno_code_in_progress);

  Network.State = network_state_connecting;
}

void UpdateNetwork() {
  Assert(Network.State != network_state_inactive);

  struct timeval Timeout;
  Timeout.tv_sec = 0;
  Timeout.tv_usec = 500;

  fd_set FDSet;
  FD_ZERO(&FDSet);
  FD_SET(FD, &FDSet);

  if(Network.State == network_state_connected) {
    int SelectResult = select(FD+1, &FDSet, NULL, NULL, &Timeout);
    if(SelectResult == -1) {
      if(errno != errno_code_interrupted_system_call) {
        InvalidCodePath;
      }
      return;
    }
    if(SelectResult != 0 && FD_ISSET(FD, &FDSet)) {
      ssize_t Result = NetworkReceive(FD, &ReceiveBuffer);
      if(Result == 0) {
        Network.State = network_state_inactive;
      }
      else {
        printf("Got something %d\n", (int)Result);
      }
    }
  }
  else {
    int SelectResult = select(FD+1, NULL, &FDSet, NULL, &Timeout);
    if(SelectResult == -1) {
      if(errno != errno_code_interrupted_system_call) {
        InvalidCodePath;
      }
      return;
    }
    if(SelectResult != 0) {
      int OptionValue;
      socklen_t OptionLength = sizeof(OptionValue);
      getsockopt(FD, SOL_SOCKET, SO_ERROR, &OptionValue, &OptionLength);
      if(OptionValue == 0) {
        Network.State = network_state_connected;
        printf("Connected.\n");
      }
      else {
        Network.State = network_state_inactive;
      }
    }
  }
}

void TerminateNetwork() {
  close(FD);
  free(ReceiveBuffer.Data);
  TerminateNetworkBuffer(&ReceiveBuffer);
  Network.State = network_state_inactive;
}
