#pragma once

#include "lib/byte_ring_buffer.h"
#include "net.h"

#define POSIX_NET_CLIENT_SET_MAX 16

struct posix_net_client {
  int FD;
  net_client_id ID;
  byte_ring_buffer InBuffer;
};

struct posix_net_client_set {
  posix_net_client Clients[POSIX_NET_CLIENT_SET_MAX];
  void *InBuffer;
  ui32 Count;
};

struct posix_net_client_set_iterator {
  posix_net_client_set *Set;
  memsize Index;
  posix_net_client *Client;
};

void InitPosixNetClientSet(posix_net_client_set *Set);
void TerminatePosixNetClientSet(posix_net_client_set *Set);
posix_net_client* CreateClient(posix_net_client_set *Set, int FD);
void DestroyClient(posix_net_client_set_iterator *Iterator);
posix_net_client_set_iterator CreatePosixNetClientSetIterator(posix_net_client_set *Set);
posix_net_client* FindClientByID(posix_net_client_set *Set, net_client_id ID);
bool AdvancePosixNetClientSetIterator(posix_net_client_set_iterator *Iterator);
