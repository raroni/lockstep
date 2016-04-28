#include "lib/assert.h"
#include "lib/buf_view.h"
#include "lib/seq_write.h"
#include "common/conversion.h"
#include "net_events.h"

static memsize ClientIDLength = sizeof(ui8);
static memsize TypeLength = sizeof(ui8);

static void WriteType(seq_write *W, net_event_type Type) {
  ui8 TypeInt = SafeCastIntToUI8(Type);
  SeqWriteUI8(W, TypeInt);
}

static net_event_type ReadType(buf_view *V) {
  ui8 TypeInt = BufViewReadUI8(V);
  return (net_event_type)TypeInt;
}

static net_client_id ReadClientID(buf_view *V) {
  ui8 IDInt = BufViewReadUI8(V);
  return (net_client_id)IDInt;
}

static void WriteClientID(seq_write *W, net_client_id ID) {
  Assert(ID < 256);
  ui8 IDUI8 = (ui8)ID;
  SeqWriteUI8(W, IDUI8);
}


buffer SerializeDisconnectNetEvent(net_client_id ID, linear_allocator *Allocator) {
  seq_write W = CreateSeqWrite(Allocator);
  WriteType(&W, net_event_type_disconnect);
  WriteClientID(&W, ID);
  return W.Buffer;
}

buffer SerializeConnectNetEvent(net_client_id ID, linear_allocator *Allocator) {
  seq_write W = CreateSeqWrite(Allocator);
  WriteType(&W, net_event_type_connect);
  WriteClientID(&W, ID);
  return W.Buffer;
}

buffer SerializeMessageNetEvent(net_client_id ID, buffer Message, linear_allocator *Allocator) {
  seq_write W = CreateSeqWrite(Allocator);

  WriteType(&W, net_event_type_message);
  WriteClientID(&W, ID);
  SeqWriteMemsize(&W, Message.Length);
  SeqWriteBuffer(&W, Message);

  return W.Buffer;
}

net_event_type UnserializeNetEventType(buffer Input) {
  buf_view V = CreateBufView(Input);
  return ReadType(&V);
}

connect_net_event UnserializeConnectNetEvent(buffer Input) {
  Assert(Input.Length == ClientIDLength + TypeLength);
  connect_net_event Event;
  buf_view V = CreateBufView(Input);
  net_event_type Type = ReadType(&V);
  Assert(Type == net_event_type_connect);
  Event.ClientID = ReadClientID(&V);
  return Event;
}

disconnect_net_event UnserializeDisconnectNetEvent(buffer Input) {
  Assert(Input.Length == ClientIDLength + TypeLength);
  disconnect_net_event Event;
  buf_view V = CreateBufView(Input);
  net_event_type Type = ReadType(&V);
  Assert(Type == net_event_type_disconnect);
  Event.ClientID = ReadClientID(&V);
  return Event;
}

message_net_event UnserializeMessageNetEvent(buffer Input) {
  buf_view V = CreateBufView(Input);

  net_event_type Type = ReadType(&V);
  Assert(Type == net_event_type_message);

  message_net_event Event;
  Event.ClientID = ReadClientID(&V);
  Event.Message.Length = BufViewReadMemsize(&V);
  Event.Message.Addr = BufViewRead(&V, Event.Message.Length);

  return Event;
}
