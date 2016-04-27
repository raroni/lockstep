#include "lib/assert.h"
#include "lib/serialization.h"
#include "net_events.h"

static memsize ClientIDLength = sizeof(ui8);
static memsize TypeLength = sizeof(ui8);

static void WriteType(serializer *S, net_event_type Type) {
  // Int casting is only to silence warning:
  // tautological-constant-out-of-range-compare
  Assert((int)Type < 256);

  ui8 TypeInt = (ui8)Type;
  SerializerWriteUI8(S, TypeInt);
}

static net_event_type ReadType(serializer *S) {
  ui8 TypeInt = SerializerReadUI8(S);
  return (net_event_type)TypeInt;
}

static net_client_id ReadClientID(serializer *S) {
  ui8 IDInt = SerializerReadUI8(S);
  return (net_client_id)IDInt;
}

static void WriteClientID(serializer *S, net_client_id ID) {
  Assert(ID < 256);
  ui8 IDInt = (ui8)ID;
  SerializerWriteUI8(S, IDInt);
}

memsize SerializeDisconnectNetEvent(net_client_id ID, buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, net_event_type_disconnect);
  WriteClientID(&S, ID);
  return S.Position;
}

memsize SerializeConnectNetEvent(net_client_id ID, buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, net_event_type_connect);
  WriteClientID(&S, ID);
  return S.Position;
}

memsize SerializeMessageNetEvent(net_client_id ID, buffer Message, buffer Out) {
  serializer S = CreateSerializer(Out);

  WriteType(&S, net_event_type_message);
  WriteClientID(&S, ID);
  SerializerWriteMemsize(&S, Message.Length);
  SerializerWriteBuffer(&S, Message);

  return S.Position;
}

net_event_type UnserializeNetEventType(buffer Input) {
  serializer S = CreateSerializer(Input);
  return ReadType(&S);
}

connect_net_event UnserializeConnectNetEvent(buffer Input) {
  Assert(Input.Length == ClientIDLength + TypeLength);
  connect_net_event Event;
  serializer S = CreateSerializer(Input);
  net_event_type Type = ReadType(&S);
  Assert(Type == net_event_type_connect);
  Event.ClientID = ReadClientID(&S);
  return Event;
}

disconnect_net_event UnserializeDisconnectNetEvent(buffer Input) {
  Assert(Input.Length == ClientIDLength + TypeLength);
  disconnect_net_event Event;
  serializer S = CreateSerializer(Input);
  net_event_type Type = ReadType(&S);
  Assert(Type == net_event_type_disconnect);
  Event.ClientID = ReadClientID(&S);
  return Event;
}

message_net_event UnserializeMessageNetEvent(buffer Input) {
  serializer S = CreateSerializer(Input);

  net_event_type Type = ReadType(&S);
  Assert(Type == net_event_type_message);

  message_net_event Event;
  Event.ClientID = ReadClientID(&S);
  Event.Message.Length = SerializerReadMemsize(&S);
  Event.Message.Addr = SerializerRead(&S, Event.Message.Length);

  return Event;
}
