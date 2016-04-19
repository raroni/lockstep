#pragma once

void InitNetwork();
void* RunNetwork(void *Data);
void TerminateNetwork();

void NetworkSend(buffer Message);
memsize ReadNetworkEvent(buffer Buffer);
void ShutdownNetwork();
