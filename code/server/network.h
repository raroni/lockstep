#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

#include "lib/def.h"

typedef memsize client_id;

void InitNetwork();
void* RunNetwork(void *Data);
void DisconnectNetwork();
void TerminateNetwork();
void NetworkBroadcast(client_id *IDs, memsize Count, buffer Message);
memsize ReadNetworkEvent(buffer Output);

#endif
