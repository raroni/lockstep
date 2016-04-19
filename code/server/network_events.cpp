#include "lib/assert.h"
#include "common/serialization.h"
#include "network_events.h"

static memsize ClientIDLength = sizeof(ui8);
static memsize TypeLength = sizeof(ui8);

static void WriteType(serializer *S, network_event_type Type) {
  // Int casting is only to silence warning:
  // tautological-constant-out-of-range-compare
  Assert((int)Type < 256);

  ui8 TypeInt = (ui8)Type;
  SerializerWriteUI8(S, TypeInt);
}

static network_event_type ReadType(serializer *S) {
  ui8 TypeInt = SerializerReadUI8(S);
  return (network_event_type)TypeInt;
}

static client_id ReadClientID(serializer *S) {
  ui8 IDInt = SerializerReadUI8(S);
  return (client_id)IDInt;
}

static void WriteClientID(serializer *S, client_id ID) {
  Assert(ID < 256);
  ui8 IDInt = (ui8)ID;
  SerializerWriteUI8(S, IDInt);
}

memsize SerializeDisconnectNetworkEvent(client_id ID, buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, network_event_type_disconnect);
  WriteClientID(&S, ID);
  return S.Position;
}

memsize SerializeConnectNetworkEvent(client_id ID, buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, network_event_type_connect);
  WriteClientID(&S, ID);
  return S.Position;
}

memsize SerializeReplyNetworkEvent(client_id ID, buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, network_event_type_reply);
  WriteClientID(&S, ID);
  return S.Position;
}

network_event_type UnserializeNetworkEventType(buffer Input) {
  serializer S = CreateSerializer(Input);
  return ReadType(&S);
}

connect_network_event UnserializeConnectNetworkEvent(buffer Input) {
  Assert(Input.Length == ClientIDLength + TypeLength);
  connect_network_event Event;
  serializer S = CreateSerializer(Input);
  network_event_type Type = ReadType(&S);
  Assert(Type == network_event_type_connect);
  Event.ClientID = ReadClientID(&S);
  return Event;
}

disconnect_network_event UnserializeDisconnectNetworkEvent(buffer Input) {
  Assert(Input.Length == ClientIDLength + TypeLength);
  disconnect_network_event Event;
  serializer S = CreateSerializer(Input);
  network_event_type Type = ReadType(&S);
  Assert(Type == network_event_type_disconnect);
  Event.ClientID = ReadClientID(&S);
  return Event;
}

reply_network_event UnserializeReplyNetworkEvent(buffer Input) {
  Assert(Input.Length == ClientIDLength + TypeLength);
  reply_network_event Event;
  serializer S = CreateSerializer(Input);
  network_event_type Type = ReadType(&S);
  Assert(Type == network_event_type_reply);
  Event.ClientID = ReadClientID(&S);
  return Event;
}
