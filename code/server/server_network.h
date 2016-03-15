#include "../shared.h"

#define CLIENT_MAX 4

struct client {
  int FD;
};

struct client_set {
  client Clients[CLIENT_MAX];
  int MaxFDPlusOne;
  ui32 Count;
};

void InitNetwork();
void UpdateNetwork();
void TerminateNetwork();
