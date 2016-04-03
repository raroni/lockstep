#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

void InitNetwork();
void* RunNetwork(void *Data);
void DisconnectNetwork();
void TerminateNetwork();

#endif
