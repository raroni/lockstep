#include "lib/assert.h"
#include "lib/serialization.h"
#include "common/conversion.h"
#include "net_messages.h"

const memsize MinMessageSize = 1;
const memsize ReplyNetMessageSize = 1;
const memsize OrderListNetMessageSize = 1;
const memsize StartNetMessageSize = 3;

void WriteType(serializer *S, net_message_type Type) {
  ui8 TypeUI8 = SafeCastIntToUI8(Type);
  SerializerWriteUI8(S, TypeUI8);
}

memsize SerializeStartNetMessage(memsize PlayerCount, memsize PlayerIndex, buffer Buffer) {
  serializer Writer = CreateSerializer(Buffer);

  ui8 TypeUI8 = SafeCastIntToUI8(net_message_type_start);
  SerializerWriteUI8(&Writer, TypeUI8);

  ui8 PlayerCountUI8 = SafeCastIntToUI8(PlayerCount);
  SerializerWriteUI8(&Writer, PlayerCountUI8);

  ui8 PlayerIndexUI8 = SafeCastIntToUI8(PlayerIndex);
  SerializerWriteUI8(&Writer, PlayerIndexUI8);

  Assert(Writer.Position == StartNetMessageSize);

  return Writer.Position;
}

bool ValidateMessageLength(buffer Buffer, net_message_type Type) {
  memsize RequiredLength = 0;
  switch(Type) {
    case net_message_type_start:
      RequiredLength = StartNetMessageSize;
      break;
    case net_message_type_reply:
      RequiredLength = ReplyNetMessageSize;
      break;
    case net_message_type_order_list:
      RequiredLength = OrderListNetMessageSize;
      break;
    default:
      InvalidCodePath;
  }

  return RequiredLength <= Buffer.Length;
}

memsize SerializeReplyNetMessage(buffer Buffer) {
  ui8 TypeInt = SafeCastIntToUI8(net_message_type_reply);
  serializer Writer = CreateSerializer(Buffer);
  SerializerWriteUI8(&Writer, TypeInt);
  Assert(Writer.Position == ReplyNetMessageSize);
  return Writer.Position;
}

memsize SerializeOrderListNetMessage(buffer Out) {
  serializer W = CreateSerializer(Out);
  WriteType(&W, net_message_type_order_list);
  Assert(W.Position == OrderListNetMessageSize);
  return W.Position;
}

net_message_type UnserializeNetMessageType(buffer Input) {
  serializer S = CreateSerializer(Input);
  net_message_type Type = (net_message_type)SerializerReadUI8(&S);
  return Type;
}

start_net_message UnserializeStartNetMessage(buffer Buffer) {
  serializer S = CreateSerializer(Buffer);
  net_message_type Type = (net_message_type)SerializerReadUI8(&S);
  Assert(Type == net_message_type_start);

  start_net_message Message;
  Message.PlayerCount = SerializerReadUI8(&S);
  Message.PlayerIndex = SerializerReadUI8(&S);

  return Message;
}

order_list_net_message UnserializeOrderListNetMessage(buffer Input) {
  serializer S = CreateSerializer(Input);
  net_message_type Type = (net_message_type)SerializerReadUI8(&S);
  Assert(Type == net_message_type_order_list);

  order_list_net_message Message;
  Message.Count = 0;

  return Message;
}

bool ValidateNetMessageType(net_message_type Type) {
  return Type < net_message_type_count;
}

bool ValidateStartNetMessage(start_net_message Message) {
  // TODO: Check properties of message
  return true;
}

bool ValidateOrderListNetMessage(order_list_net_message Message) {
  // TODO: Check properties of message
  return true;
}
