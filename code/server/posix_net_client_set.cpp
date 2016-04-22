#include <stdlib.h>
#include "posix_net_client_set.h"

typedef posix_net_client_set set;
typedef posix_net_client_set_iterator iterator;

static client_id CreateClientID() {
  static client_id DummyHandleCount = 10;
  return DummyHandleCount++;
}

void InitPosixNetClientSet(set *Set) {
  Set->Count = 0;
  memsize InBufferTotalCapacity = POSIX_NET_CLIENT_SET_MAX*1024*50;
  memsize InBufferClientCapacity = InBufferTotalCapacity/POSIX_NET_CLIENT_SET_MAX;
  Set->InBuffer = malloc(InBufferTotalCapacity);
  for(memsize I=0; I<POSIX_NET_CLIENT_SET_MAX; ++I) {
    buffer Storage;
    Storage.Addr = ((ui8*)Set->InBuffer) + I*InBufferClientCapacity;
    Storage.Length = InBufferClientCapacity;
    InitByteRingBuffer(&Set->Clients[I].InBuffer, Storage);
  }
}

posix_net_client* FindClientByID(posix_net_client_set *Set, client_id ID) {
  for(memsize I=0; I<Set->Count; ++I) {
    posix_net_client *Client = Set->Clients + I;
    if(Client->ID == ID) {
      return Client;
    }
  }
  return NULL;
}

posix_net_client* CreateClient(set *Set, int FD) {
  posix_net_client *Client = &Set->Clients[Set->Count++];
  Client->FD = FD;
  Client->ID = CreateClientID();
  return Client;
}

posix_net_client_set_iterator CreatePosixNetClientSetIterator(set *Set) {
  iterator Iterator;
  Iterator.Set = Set;
  Iterator.Client = Set->Clients - 1;
  return Iterator;
}

void DestroyClient(posix_net_client_set_iterator *Iterator) {
  set *Set = Iterator->Set;
  memsize Index = Iterator->Client - Set->Clients;
  Set->Clients[Index] = Set->Clients[Set->Count-1];
  Set->Count--;
  Iterator->Client--;
}

bool AdvancePosixNetClientSetIterator(iterator *Iterator) {
  posix_net_client_set *Set = Iterator->Set;
  Iterator->Client++;
  return Set->Clients + Set->Count > Iterator->Client;
}

void TerminatePosixNetClientSet(posix_net_client_set *Set) {
  for(memsize I=0; I<Set->Count; ++I) {
    TerminateByteRingBuffer(&Set->Clients[I].InBuffer);
  }

  free(Set->InBuffer);
  Set->InBuffer = NULL;
  Set->Count = 0;
}
