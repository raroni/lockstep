#include "lib/assert.h"
#include "common/conversion.h"
#include "serialization.h"
#include "network_messages.h"

const memsize StartNetworkMesageSize = 1;
const memsize ReplyNetworkMesageSize = 1;

memsize SerializeStartNetworkMessage(buffer Buffer) {
  ui8 TypeInt = SafeCastIntToUI8(network_message_type_start);
  serializer Writer = CreateSerializer(Buffer);
  SerializerWriteUI8(&Writer, TypeInt);
  Assert(Writer.Position == StartNetworkMesageSize);
  return Writer.Position;
}

memsize SerializeReplyNetworkMessage(buffer Buffer) {
  ui8 TypeInt = SafeCastIntToUI8(network_message_type_reply);
  serializer Writer = CreateSerializer(Buffer);
  SerializerWriteUI8(&Writer, TypeInt);
  Assert(Writer.Position == ReplyNetworkMesageSize);
  return Writer.Position;
}

bool UnserializeNetworkMessageType(buffer Input, network_message_type *Type) {
  if(Input.Length >= 1) {
    ui8 TypeInt = *(ui8*)Input.Addr;
    *Type = (network_message_type)TypeInt;
    return true;
  }
  else {
    return false;
  }
}
