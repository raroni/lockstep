#ifndef CLIENT_NETWORK
#define CLIENT_NETWORK

void InitNetwork();
void* RunNetwork(void *Data);
void ShutdownNetwork();
memsize ReadNetworkEvent(buffer Buffer);
void TerminateNetwork();

#endif
