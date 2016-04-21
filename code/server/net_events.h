#pragma once

#include "net.h"

#define NETWORK_EVENT_MAX_LENGTH 512

enum net_event_type {
  net_event_type_connect,
  net_event_type_disconnect,
  net_event_type_reply
};

struct connect_net_event {
  client_id ClientID;
};

struct disconnect_net_event {
  client_id ClientID;
};

struct reply_net_event {
  client_id ClientID;
};

memsize SerializeDisconnectNetEvent(client_id ID, buffer Out);
memsize SerializeConnectNetEvent(client_id ID, buffer Out);
memsize SerializeReplyNetEvent(client_id ID, buffer Out);
net_event_type UnserializeNetEventType(buffer Input);
connect_net_event UnserializeConnectNetEvent(buffer Input);
disconnect_net_event UnserializeDisconnectNetEvent(buffer Input);
reply_net_event UnserializeReplyNetEvent(buffer Input);
