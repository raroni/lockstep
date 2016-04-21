#include "lib/assert.h"
#include "lib/serialization.h"
#include "common/conversion.h"
#include "net_messages.h"

const memsize StartNetMesageSize = 1;
const memsize ReplyNetMesageSize = 1;

memsize SerializeStartNetMessage(buffer Buffer) {
  ui8 TypeInt = SafeCastIntToUI8(net_message_type_start);
  serializer Writer = CreateSerializer(Buffer);
  SerializerWriteUI8(&Writer, TypeInt);
  Assert(Writer.Position == StartNetMesageSize);
  return Writer.Position;
}

memsize SerializeReplyNetMessage(buffer Buffer) {
  ui8 TypeInt = SafeCastIntToUI8(net_message_type_reply);
  serializer Writer = CreateSerializer(Buffer);
  SerializerWriteUI8(&Writer, TypeInt);
  Assert(Writer.Position == ReplyNetMesageSize);
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
