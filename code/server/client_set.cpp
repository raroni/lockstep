#include "client_set.h"

typedef client_set set;
typedef client_set_iterator iterator;

static client_id CreateClientID() {
  static client_id DummyHandleCount = 0;
  return DummyHandleCount++;
}

void InitClientSet(set *Set) {
  Set->Count = 0;
}

void CreateClient(set *Set, int FD) {
  client *Client = &Set->Clients[Set->Count++];
  Client->FD = FD;
  Client->ID = CreateClientID();
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
