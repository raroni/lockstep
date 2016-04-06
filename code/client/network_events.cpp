#include "lib/assert.h"
#include "common/serialization.h"
#include "network_events.h"

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

memsize SerializeConnectionEstablishedNetworkEvent(buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, network_event_type_connection_established);
  return S.Position;
}

memsize SerializeConnectionLostNetworkEvent(buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, network_event_type_connection_lost);
  return S.Position;
}

memsize SerializeConnectionFailedNetworkEvent(buffer Out) {
  serializer S = CreateSerializer(Out);
  WriteType(&S, network_event_type_connection_failed);
  return S.Position;
}

network_event_type UnserializeNetworkEventType(buffer Input) {
  serializer S = CreateSerializer(Input);
  return ReadType(&S);
}
