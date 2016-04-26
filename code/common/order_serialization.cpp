#include "lib/serialization.h"
#include "common/conversion.h"
#include "order_serialization.h"

buffer SerializeOrder(simulation_order Order, linear_allocator *Allocator) {
  memsize BufferLength = (4 + Order.UnitCount) * sizeof(ui16);
  buffer Buffer = {
    .Addr = LinearAllocate(Allocator, BufferLength),
    .Length = BufferLength
  };
  serializer W = CreateSerializer(Buffer);

  ui16 PlayerIDUI16 = SafeCastIntToUI16(Order.PlayerID);

  SerializerWriteUI16(&W, PlayerIDUI16);
  SerializerWriteUI16(&W, Order.UnitCount);
  SerializerWriteSI16(&W, Order.Target.X);
  SerializerWriteSI16(&W, Order.Target.Y);
  for(memsize I=0; I<Order.UnitCount; ++I) {
    ui16 UnitIDUI16 = SafeCastIntToUI16(Order.UnitIDs[I]);
    SerializerWriteUI16(&W, UnitIDUI16);
  }

  return Buffer;
}

simulation_order UnserializeOrder(buffer OrderBuffer, linear_allocator *Allocator) {
  simulation_order Order;

  serializer W = CreateSerializer(OrderBuffer);
  Order.PlayerID = SerializerReadUI16(&W);
  Order.UnitCount = SerializerReadUI16(&W);
  Order.Target.X = SerializerReadSI16(&W);
  Order.Target.Y = SerializerReadSI16(&W);

  memsize UnitIDsSize = sizeof(simulation_unit_id) * Order.UnitCount;
  Order.UnitIDs = (simulation_unit_id*)LinearAllocate(Allocator, UnitIDsSize);

  for(memsize I=0; I<Order.UnitCount; ++I) {
    Order.UnitIDs[I] = SerializerReadSI16(&W);
  }

  return Order;
}
