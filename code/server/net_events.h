#pragma once

#include "net.h"

#define NET_EVENT_MAX_LENGTH 512

enum net_event_type {
  net_event_type_connect,
  net_event_type_disconnect,
  net_event_type_message
};

struct connect_net_event {
  net_client_id ClientID;
};

struct disconnect_net_event {
  net_client_id ClientID;
};


struct message_net_event {
  net_client_id ClientID;
  buffer Message;
};

memsize SerializeDisconnectNetEvent(net_client_id ID, buffer Out);
memsize SerializeConnectNetEvent(net_client_id ID, buffer Out);
memsize SerializeMessageNetEvent(net_client_id ID, buffer Message, buffer Out);
net_event_type UnserializeNetEventType(buffer Input);
connect_net_event UnserializeConnectNetEvent(buffer Input);
disconnect_net_event UnserializeDisconnectNetEvent(buffer Input);
message_net_event UnserializeMessageNetEvent(buffer Input);
