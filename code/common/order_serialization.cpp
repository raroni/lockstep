#include "common/conversion.h"
#include "order_serialization.h"
#include "lib/seq_write.h"

static void WriterOrder(simulation_order Order, seq_write *W) {
  ui16 PlayerIDUI8 = SafeCastIntToUI8(Order.PlayerID);

  SeqWriteUI8(W, PlayerIDUI8);
  SeqWriteUI16(W, Order.UnitCount);
  SeqWriteSI16(W, Order.Target.X);
  SeqWriteSI16(W, Order.Target.Y);
  for(memsize I=0; I<Order.UnitCount; ++I) {
    ui16 UnitIDUI16 = SafeCastIntToUI16(Order.UnitIDs[I]);
    SeqWriteUI16(W, UnitIDUI16);
  }
}

static simulation_order ReadOrder(buf_view *V, memory_arena *Arena) {
  simulation_order Order;
  Order.PlayerID = BufViewReadUI8(V);
  Order.UnitCount = BufViewReadUI16(V);
  Order.Target.X = BufViewReadSI16(V);
  Order.Target.Y = BufViewReadSI16(V);

  memsize UnitIDsSize = sizeof(simulation_unit_id) * Order.UnitCount;
  Order.UnitIDs = (simulation_unit_id*)MemoryArenaAllocate(Arena, UnitIDsSize);

  for(memsize I=0; I<Order.UnitCount; ++I) {
    Order.UnitIDs[I] = BufViewReadSI16(V);
  }

  return Order;
}

buffer SerializeOrder(simulation_order Order, memory_arena *Arena) {
  seq_write W = CreateSeqWrite(Arena);
  WriterOrder(Order, &W);
  return W.Buffer;
}

simulation_order UnserializeOrder(buffer OrderBuffer, memory_arena *Arena) {
  buf_view W = CreateBufView(OrderBuffer);
  return ReadOrder(&W, Arena);
}

buffer SerializeOrderList(simulation_order_list *List, memory_arena *Arena) {
  seq_write W = CreateSeqWrite(Arena);
  SeqWriteUI16(&W, List->Count);

  for(memsize I=0; I<List->Count; ++I) {
    WriterOrder(List->Orders[I], &W);
  }

  return W.Buffer;
}

simulation_order_list UnserializeOrderList(buffer ListBuffer, memory_arena *Arena) {
  buf_view R = CreateBufView(ListBuffer);
  simulation_order_list List;
  List.Count = BufViewReadUI16(&R);

  if(List.Count != 0) {
    memsize OrdersSize = sizeof(simulation_order) * List.Count;
    List.Orders = (simulation_order*)MemoryArenaAllocate(Arena, OrdersSize);

    for(memsize I=0; I<List.Count; ++I) {
      List.Orders[I] = ReadOrder(&R, Arena);
    }
  }
  else {
    List.Orders = NULL;
  }

  return List;
}
