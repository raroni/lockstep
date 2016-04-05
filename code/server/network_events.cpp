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

network_event_type UnserializeNetworkEventType(void *Buffer, memsize Length) {
  serializer S = CreateSerializer(Buffer, Length);
  return ReadType(&S);
}

memsize SerializeDisconnectNetworkEvent(client_id ID, void *Buffer, memsize Length) {
  serializer S = CreateSerializer(Buffer, Length);
  WriteType(&S, network_event_type_disconnect);
  WriteClientID(&S, ID);
  return S.Position;
}

memsize SerializeConnectNetworkEvent(client_id ID, void *Buffer, memsize Length) {
  serializer S = CreateSerializer(Buffer, Length);
  WriteType(&S, network_event_type_connect);
  WriteClientID(&S, ID);
  return S.Position;
}

connect_network_event UnserializeConnectNetworkEvent(void *Buffer, memsize Length) {
  Assert(Length == ClientIDLength + TypeLength);
  connect_network_event Event;
  serializer S = CreateSerializer(Buffer, Length);
  network_event_type Type = ReadType(&S);
  Assert(Type == network_event_type_connect);
  Event.ClientID = ReadClientID(&S);
  return Event;
}

disconnect_network_event UnserializeDisconnectNetworkEvent(void *Buffer, memsize Length) {
  Assert(Length == ClientIDLength + TypeLength);
  disconnect_network_event Event;
  serializer S = CreateSerializer(Buffer, Length);
  network_event_type Type = ReadType(&S);
  Assert(Type == network_event_type_disconnect);
  Event.ClientID = ReadClientID(&S);
  return Event;
}
