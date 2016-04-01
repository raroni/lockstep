#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include "server_network.h"
#include "../assert.h"

static int HostFD;
network Network;

#define TEST_BUFFER_SIZE 4096
ui8 TestBuffer[TEST_BUFFER_SIZE];

void UpdateClientSet(client_set *Set) {
  Set->MaxFDPlusOne = 0;
  for(ui32 I=0; I<Set->Count; ++I) {
    Set->MaxFDPlusOne = MaxInt(Set->MaxFDPlusOne, Set->Clients[I].FD + 1);
  }
}

void RemoveClient(client_set *Set, ui32 Index) {
  Set->Clients[Index] = Set->Clients[Set->Count-1];
  Set->Count--;
  UpdateClientSet(Set);
}

void InitNetwork() {
  Network.ClientSet.Count = 0;
  Network.ClientSet.MaxFDPlusOne = 0;

  HostFD = socket(PF_INET, SOCK_STREAM, 0);
  Assert(HostFD != -1);
  fcntl(HostFD, F_SETFL, O_NONBLOCK);

  struct sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_len = sizeof(Address);
  Address.sin_family = AF_INET;
  Address.sin_port = htons(4321);
  Address.sin_addr.s_addr = INADDR_ANY;

  int BindResult = bind(HostFD, (struct sockaddr *)&Address, sizeof(Address));
  Assert(BindResult != -1);

  int ListenResult = listen(HostFD, 5);
  Assert(ListenResult == 0);
}

void DisconnectNetwork() {
  for(ui32 I=0; I<Network.ClientSet.Count; ++I) {
    shutdown(Network.ClientSet.Clients[I].FD, SHUT_RDWR);
  }
}

void UpdateNetwork() {
  fd_set ClientFDSet;
  FD_ZERO(&ClientFDSet);
  for(ui32 I=0; I<Network.ClientSet.Count; ++I) {
    FD_SET(Network.ClientSet.Clients[I].FD, &ClientFDSet);
  }

  struct timeval Timeout;
  Timeout.tv_sec = 0;
  Timeout.tv_usec = 5000;

  int SelectResult = select(Network.ClientSet.MaxFDPlusOne, &ClientFDSet, NULL, NULL, &Timeout);
  if(SelectResult == -1) {
    if(errno == errno_code_interrupted_system_call) {
      return;
    }
    InvalidCodePath;
  }
  else if(SelectResult != 0) {
    for(ui32 I=0; I<Network.ClientSet.Count; ++I) {
      client *Client = Network.ClientSet.Clients + I;
      if(FD_ISSET(Client->FD, &ClientFDSet)) {
        ssize_t Result = recv(Client->FD, TestBuffer, TEST_BUFFER_SIZE, 0); // TODO: Loop until you have all
        if(Result == 0) {
          int Result = close(Network.ClientSet.Clients[I].FD);
          Assert(Result == 0);
          RemoveClient(&Network.ClientSet, I);
          I--;
          printf("Disconnected.\n");
        }
        else {
          printf("Got something\n");
        }
      }
    }
  }

  int ClientFD = accept(HostFD, NULL, NULL);
  if(ClientFD != -1) {
    Network.ClientSet.Clients[0].FD = ClientFD;
    Network.ClientSet.Count++;
    Network.ClientSet.MaxFDPlusOne = MaxInt(ClientFD + 1, Network.ClientSet.MaxFDPlusOne);
    printf("Someone connected!\n");
  }
}

void NetworkBroadcast(void *Data, size_t Length) {
  for(ui32 I=0; I<Network.ClientSet.Count; ++I) {
    client *Client = Network.ClientSet.Clients + I;
    int Result = send(Client->FD, Data, Length, 0);
    Assert(Result != -1);
  }
}

void TerminateNetwork() {
  close(HostFD);
}
