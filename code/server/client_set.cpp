#include <stdlib.h>
#include "client_set.h"

typedef client_set set;
typedef client_set_iterator iterator;

static client_id CreateClientID() {
  static client_id DummyHandleCount = 10;
  return DummyHandleCount++;
}

void InitClientSet(set *Set) {
  Set->Count = 0;
  memsize InBufferTotalCapacity = CLIENT_SET_MAX*1024*50;
  memsize InBufferClientCapacity = InBufferTotalCapacity/CLIENT_SET_MAX;
  Set->InBuffer = malloc(InBufferTotalCapacity);
  for(memsize I=0; I<Set->Count; ++I) {
    void *Data = ((ui8*)Set->InBuffer) + I*InBufferClientCapacity;
    InitByteRingBuffer(&Set->Clients[I].InBuffer, Data, InBufferClientCapacity);
  }
}

client* FindClientByID(client_set *Set, client_id ID) {
  for(memsize I=0; I<Set->Count; ++I) {
    client *Client = Set->Clients + I;
    if(Client->ID == ID) {
      return Client;
    }
  }
  return NULL;
}

client* CreateClient(set *Set, int FD) {
  client *Client = &Set->Clients[Set->Count++];
  Client->FD = FD;
  Client->ID = CreateClientID();
  return Client;
}

client_set_iterator CreateClientSetIterator(set *Set) {
  iterator Iterator;
  Iterator.Set = Set;
  Iterator.Client = Set->Clients - 1;
  return Iterator;
}

void DestroyClient(client_set_iterator *Iterator) {
  set *Set = Iterator->Set;
  memsize Index = Iterator->Client - Set->Clients;
  Set->Clients[Index] = Set->Clients[Set->Count-1];
  Set->Count--;
  Iterator->Client--;
}

bool AdvanceClientSetIterator(iterator *Iterator) {
  client_set *Set = Iterator->Set;
  Iterator->Client++;
  return Set->Clients + Set->Count > Iterator->Client;
}

void TerminateClientSet(client_set *Set) {
  for(memsize I=0; I<Set->Count; ++I) {
    TerminateByteRingBuffer(&Set->Clients[I].InBuffer);
  }

  free(Set->InBuffer);
  Set->InBuffer = NULL;
  Set->Count = 0;
}
