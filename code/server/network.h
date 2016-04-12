#pragma once

#include "lib/def.h"

typedef memsize client_id;

void InitNetwork();
void* RunNetwork(void *Data);
void ShutdownNetwork();
void TerminateNetwork();
void NetworkBroadcast(client_id *IDs, memsize Count, buffer Message);
memsize ReadNetworkEvent(buffer Output);
