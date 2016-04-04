#ifndef SERVER_CLIENT_SET_H
#define SERVER_CLIENT_SET_H

#include "lib/byte_ring_buffer.h"
#include "network.h"

#define CLIENT_SET_MAX 16

struct client {
  int FD;
  client_id ID;
  byte_ring_buffer InBuffer;
};

struct client_set {
  client Clients[CLIENT_SET_MAX];
  void *InBuffer;
  ui32 Count;
};

struct client_set_iterator {
  client_set *Set;
  memsize Index;
  client *Client;
};

void InitClientSet(client_set *Set);
void TerminateClientSet(client_set *Set);
client* CreateClient(client_set *Set, int FD);
void DestroyClient(client_set_iterator *Iterator);
client_set_iterator CreateClientSetIterator(client_set *Set);
client* FindClientByID(client_set *Set, client_id ID);
bool AdvanceClientSetIterator(client_set_iterator *Iterator);

#endif
