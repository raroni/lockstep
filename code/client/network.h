#pragma once

void InitNetwork();
void* RunNetwork(void *Data);
void ShutdownNetwork();
memsize ReadNetworkEvent(buffer Buffer);
void NetworkSend(buffer Message);
void TerminateNetwork();
