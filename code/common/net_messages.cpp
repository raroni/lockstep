#include "lib/assert.h"
#include "lib/serialization.h"
#include "lib/data_writer.h"
#include "common/conversion.h"
#include "net_messages.h"

void WriteType(serializer *S, net_message_type Type) {
  ui8 TypeUI8 = SafeCastIntToUI8(Type);
  SerializerWriteUI8(S, TypeUI8);
}

void WriteType(data_writer *W, net_message_type Type) {
  ui8 TypeUI8 = SafeCastIntToUI8(Type);
  WriteUI8(W, TypeUI8);
}

static order_net_message UnserializeOrderHeader(serializer *S) {
  net_message_type Type = (net_message_type)SerializerReadUI8(S);
  Assert(Type == net_message_type_order);

  order_net_message Message;
  Message.UnitCount = SerializerReadUI16(S);
  Message.Target.X = SerializerReadSI16(S);
  Message.Target.Y = SerializerReadSI16(S);
  Message.UnitIDs = NULL;

  return Message;
}

buffer SerializeStartNetMessage(memsize PlayerCount, memsize PlayerIndex, linear_allocator *Allocator) {
  data_writer W = CreateDataWriter(Allocator);

  ui8 TypeUI8 = SafeCastIntToUI8(net_message_type_start);
  WriteUI8(&W, TypeUI8);

  ui8 PlayerCountUI8 = SafeCastIntToUI8(PlayerCount);
  WriteUI8(&W, PlayerCountUI8);

  ui8 PlayerIndexUI8 = SafeCastIntToUI8(PlayerIndex);
  WriteUI8(&W, PlayerIndexUI8);

  return W.Buffer;
}

buffer SerializeReplyNetMessage(linear_allocator *Allocator) {
  ui8 TypeInt = SafeCastIntToUI8(net_message_type_reply);
  data_writer W = CreateDataWriter(Allocator);
  WriteUI8(&W, TypeInt);
  return W.Buffer;
}

buffer SerializeOrderListNetMessage(net_message_order *Orders, ui16 OrderCount, linear_allocator *Allocator) {
  data_writer W = CreateDataWriter(Allocator);
  WriteType(&W, net_message_type_order_list);
  WriteUI16(&W, OrderCount);

  for(memsize I=0; I<OrderCount; ++I) {
    net_message_order *O = Orders + I;
    WriteUI8(&W, O->PlayerID);
    WriteUI16(&W, O->UnitCount);
    WriteUI16(&W, O->Target.X);
    WriteUI16(&W, O->Target.Y);

    for(memsize U=0; U<O->UnitCount; ++U) {
      WriteUI16(&W, O->UnitIDs[U]);
    }
  }

  return W.Buffer;
}

buffer SerializeOrderNetMessage(ui16 *UnitIDs, memsize UnitCount, ivec2 Target, linear_allocator *Allocator) {
  Assert(GetLinearAllocatorFree(Allocator) >= NET_MESSAGE_MAX_LENGTH);

  data_writer W = CreateDataWriter(Allocator);
  WriteType(&W, net_message_type_order);
  ui16 UnitCountUI16 = SafeCastIntToUI16(UnitCount);
  WriteUI16(&W, UnitCountUI16);

  WriteSI16(&W, Target.X);
  WriteSI16(&W, Target.Y);

  for(memsize I=0; I<UnitCount; ++I) {
    ui16 IDUI16 = SafeCastIntToUI16(UnitIDs[I]);
    WriteUI16(&W, IDUI16);
  }

  Assert(W.Buffer.Length <= NET_MESSAGE_MAX_LENGTH);

  return W.Buffer;
}

net_message_type UnserializeNetMessageType(buffer Input) {
  serializer S = CreateSerializer(Input);
  net_message_type Type = (net_message_type)SerializerReadUI8(&S);
  return Type;
}

order_net_message UnserializeOrderNetMessage(buffer Input, linear_allocator *Allocator) {
  serializer S = CreateSerializer(Input);
  order_net_message Message = UnserializeOrderHeader(&S);

  memsize IDsSize = sizeof(ui16) * Message.UnitCount;
  Message.UnitIDs = (ui16*)LinearAllocate(Allocator, IDsSize);
  for(memsize I=0; I<Message.UnitCount; ++I) {
    Message.UnitIDs[I] = SerializerReadUI16(&S);
  }

  return Message;
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

order_list_net_message UnserializeOrderListNetMessage(buffer Input, linear_allocator *Allocator) {
  serializer S = CreateSerializer(Input);
  net_message_type Type = (net_message_type)SerializerReadUI8(&S);
  Assert(Type == net_message_type_order_list);

  order_list_net_message Message;
  Message.Count = SerializerReadUI16(&S);

  if(Message.Count != 0) {
    memsize OrdersSize = sizeof(net_message_order) * Message.Count;
    Message.Orders = (net_message_order*)LinearAllocate(Allocator, OrdersSize);

    for(memsize I=0; I<Message.Count; ++I) {
      net_message_order *O = Message.Orders + I;
      O->PlayerID = SerializerReadUI8(&S);
      O->UnitCount = SerializerReadUI16(&S);
      O->Target.X = SerializerReadUI16(&S);
      O->Target.Y = SerializerReadUI16(&S);

      memsize UnitIDsSize = sizeof(ui16) * O->UnitCount;
      O->UnitIDs = (ui16*)LinearAllocate(Allocator, UnitIDsSize);
      for(memsize U=0; U<O->UnitCount; ++U) {
        O->UnitIDs[U] = SerializerReadUI16(&S);
      }
    }
  }
  else {
    Message.Orders = NULL;
  }

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

bool ValidateOrderNetMessage(order_net_message Message) {
  // TODO: Check properties of message
  return true;
}
