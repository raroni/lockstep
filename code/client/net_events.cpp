#include "lib/assert.h"
#include "lib/serialization.h"
#include "common/conversion.h"
#include "net_events.h"

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

memsize SerializeConnectionEstablishedNetEvent(buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, net_event_type_connection_established);
  return S.Position;
}

memsize SerializeConnectionLostNetEvent(buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, net_event_type_connection_lost);
  return S.Position;
}

memsize SerializeConnectionFailedNetEvent(buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, net_event_type_connection_failed);
  return S.Position;
}

net_event_type UnserializeNetEventType(buffer Input) {
  serializer S = CreateSerializer(Input);
  return ReadType(&S);
}

memsize SerializeStartNetEvent(buffer Out, memsize PlayerCount, memsize PlayerID) {
  serializer S = CreateSerializer(Out);

  WriteType(&S, net_event_type_start);

  ui8 PlayerCountInt = SafeCastIntToUI8(PlayerCount);
  SerializerWriteUI8(&S, PlayerCountInt);

  ui8 PlayerIDInt = SafeCastIntToUI8(PlayerID);
  SerializerWriteUI8(&S, PlayerIDInt);

  return S.Position;
}

start_net_event UnserializeStartNetEvent(buffer Event) {
  serializer S = CreateSerializer(Event);

  net_event_type Type = ReadType(&S);
  Assert(Type == net_event_type_start);

  start_net_event StartEvent;
  StartEvent.PlayerCount = SerializerReadUI8(&S);
  StartEvent.PlayerID = SerializerReadUI8(&S);

  return StartEvent;
}
