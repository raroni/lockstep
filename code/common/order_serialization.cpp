#include "common/conversion.h"
#include "order_serialization.h"

void SerializeOrderX(simulation_order Order, serializer *W, linear_allocator *Allocator) {
  ui16 PlayerIDUI16 = SafeCastIntToUI16(Order.PlayerID);

  SerializerWriteUI16(W, PlayerIDUI16);
  SerializerWriteUI16(W, Order.UnitCount);
  SerializerWriteSI16(W, Order.Target.X);
  SerializerWriteSI16(W, Order.Target.Y);
  for(memsize I=0; I<Order.UnitCount; ++I) {
    ui16 UnitIDUI16 = SafeCastIntToUI16(Order.UnitIDs[I]);
    SerializerWriteUI16(W, UnitIDUI16);
  }
}

simulation_order UnserializeOrderX(serializer *S, linear_allocator *Allocator) {
  simulation_order Order;
  Order.PlayerID = SerializerReadUI16(S);
  Order.UnitCount = SerializerReadUI16(S);
  Order.Target.X = SerializerReadSI16(S);
  Order.Target.Y = SerializerReadSI16(S);

  memsize UnitIDsSize = sizeof(simulation_unit_id) * Order.UnitCount;
  Order.UnitIDs = (simulation_unit_id*)LinearAllocate(Allocator, UnitIDsSize);

  for(memsize I=0; I<Order.UnitCount; ++I) {
    Order.UnitIDs[I] = SerializerReadSI16(S);
  }

  return Order;
}

buffer SerializeOrder(simulation_order Order, linear_allocator *Allocator) {
  memsize BufferLength = (4 + Order.UnitCount) * sizeof(ui16);
  buffer Buffer = {
    .Addr = LinearAllocate(Allocator, BufferLength),
    .Length = BufferLength
  };
  serializer W = CreateSerializer(Buffer);
  SerializeOrderX(Order, &W, Allocator);

  return Buffer;
}

simulation_order UnserializeOrder(buffer OrderBuffer, linear_allocator *Allocator) {
  serializer W = CreateSerializer(OrderBuffer);
  return UnserializeOrderX(&W, Allocator);
}

memsize SerializeOrderList(simulation_order_list *List, buffer Output) {
  serializer W = CreateSerializer(Output);
  SerializerWriteUI16(&W, List->Count);

  for(memsize I=0; I<List->Count; ++I) {
    simulation_order *O = List->Orders + I;
    SerializerWriteUI8(&W, O->PlayerID);
    SerializerWriteUI16(&W, O->UnitCount);
    SerializerWriteUI16(&W, O->Target.X);
    SerializerWriteUI16(&W, O->Target.Y);

    for(memsize U=0; U<O->UnitCount; ++U) {
      SerializerWriteUI16(&W, O->UnitIDs[U]);
    }
  }

  return W.Position;
}

simulation_order_list UnserializeOrderList(buffer ListBuffer, linear_allocator *Allocator) {
  serializer R = CreateSerializer(ListBuffer);
  simulation_order_list List;
  List.Count = SerializerReadUI16(&R);

  if(List.Count != 0) {
    memsize OrdersSize = sizeof(simulation_order) * List.Count;
    List.Orders = (simulation_order*)LinearAllocate(Allocator, OrdersSize);

    for(memsize I=0; I<List.Count; ++I) {
      simulation_order *O = List.Orders + I;
      O->PlayerID = SerializerReadUI8(&R);
      O->UnitCount = SerializerReadUI16(&R);
      O->Target.X = SerializerReadUI16(&R);
      O->Target.Y = SerializerReadUI16(&R);

      memsize UnitIDsSize = sizeof(ui16) * O->UnitCount;
      O->UnitIDs = (ui16*)LinearAllocate(Allocator, UnitIDsSize);
      for(memsize U=0; U<O->UnitCount; ++U) {
        O->UnitIDs[U] = SerializerReadUI16(&R);
      }
    }
  }
  else {
    List.Orders = NULL;
  }

  return List;
}
