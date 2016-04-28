#include "lib/assert.h"
#include "lib/serialization.h"
#include "lib/seq_write.h"
#include "common/conversion.h"
#include "net_events.h"

static void WriteType(seq_write *W, net_event_type Type) {
  ui8 TypeInt = SafeCastIntToUI8(Type);
  SeqWriteUI8(W, TypeInt);
}

static net_event_type ReadType(serializer *S) {
  ui8 TypeInt = SerializerReadUI8(S);
  return (net_event_type)TypeInt;
}

buffer SerializeConnectionEstablishedNetEvent(linear_allocator *Allocator) {
  seq_write W = CreateSeqWrite(Allocator);
  WriteType(&W, net_event_type_connection_established);
  return W.Buffer;
}

buffer SerializeConnectionLostNetEvent(linear_allocator *Allocator) {
  seq_write W = CreateSeqWrite(Allocator);
  WriteType(&W, net_event_type_connection_lost);
  return W.Buffer;
}

buffer SerializeConnectionFailedNetEvent(linear_allocator *Allocator) {
  seq_write W = CreateSeqWrite(Allocator);
  WriteType(&W, net_event_type_connection_failed);
  return W.Buffer;
}

net_event_type UnserializeNetEventType(buffer Input) {
  serializer S = CreateSerializer(Input);
  return ReadType(&S);
}

buffer SerializeMessageNetEvent(buffer Message, linear_allocator *Allocator) {
  seq_write W = CreateSeqWrite(Allocator);

  WriteType(&W, net_event_type_message);
  SeqWriteMemsize(&W, Message.Length);
  SeqWriteBuffer(&W, Message);

  return W.Buffer;
}

message_net_event UnserializeMessageNetEvent(buffer Event) {
  serializer S = CreateSerializer(Event);

  net_event_type Type = ReadType(&S);
  Assert(Type == net_event_type_message);

  message_net_event MessageEvent;
  MessageEvent.Message.Length = SerializerReadMemsize(&S);
  MessageEvent.Message.Addr = SerializerRead(&S, MessageEvent.Message.Length);

  return MessageEvent;
}
