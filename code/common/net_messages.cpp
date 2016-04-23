#include "lib/assert.h"
#include "lib/serialization.h"
#include "common/conversion.h"
#include "net_messages.h"

const memsize ReplyNetMessageSize = 1;
const memsize StartNetMessageSize = 3;

memsize SerializeStartNetMessage(memsize PlayerCount, memsize PlayerID, buffer Buffer) {
  serializer Writer = CreateSerializer(Buffer);

  ui8 TypeUI8 = SafeCastIntToUI8(net_message_type_start);
  SerializerWriteUI8(&Writer, TypeUI8);

  ui8 PlayerCountUI8 = SafeCastIntToUI8(PlayerCount);
  SerializerWriteUI8(&Writer, PlayerCountUI8);

  ui8 PlayerIDUI8 = SafeCastIntToUI8(PlayerID);
  SerializerWriteUI8(&Writer, PlayerIDUI8);

  Assert(Writer.Position == StartNetMessageSize);

  return Writer.Position;
}

memsize SerializeReplyNetMessage(buffer Buffer) {
  ui8 TypeInt = SafeCastIntToUI8(net_message_type_reply);
  serializer Writer = CreateSerializer(Buffer);
  SerializerWriteUI8(&Writer, TypeInt);
  Assert(Writer.Position == ReplyNetMessageSize);
  return Writer.Position;
}

bool UnserializeNetMessageType(buffer Input, net_message_type *Type) {
  if(Input.Length >= 1) {
    ui8 TypeInt = *(ui8*)Input.Addr;
    *Type = (net_message_type)TypeInt;
    return true;
  }
  else {
    return false;
  }
}

bool UnserializeStartNetMessage(buffer Buffer, start_net_message *Message) {
  if(Buffer.Length < StartNetMessageSize) {
    return false;
  }

  serializer S = CreateSerializer(Buffer);
  net_message_type Type = (net_message_type)SerializerReadUI8(&S);
  Assert(Type == net_message_type_start);

  Message->PlayerCount = SerializerReadUI8(&S);
  Message->PlayerID = SerializerReadUI8(&S);

  return true;
}
