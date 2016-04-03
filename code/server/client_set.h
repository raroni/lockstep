#ifndef SERVER_CLIENT_SET_H
#define SERVER_CLIENT_SET_H

#include "../lib/def.h"

#define CLIENT_SET_MAX 64

typedef memsize client_id;

struct client {
  int FD;
  client_id ID;
};

struct client_set {
  client Clients[CLIENT_SET_MAX];
  ui32 Count;
};

struct client_set_iterator {
  client_set *Set;
  memsize Index;
  client *Client;
};

void InitClientSet(client_set *Set);
void CreateClient(client_set *Set, int FD);
void DestroyClient(client_set *Set, client_id ID);
client_set_iterator CreateClientSetIterator(client_set *Set);
bool AdvanceClientSetIterator(client_set_iterator *Iterator);

#endif
