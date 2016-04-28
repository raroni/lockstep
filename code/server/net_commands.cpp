#include "lib/assert.h"
#include "lib/serialization.h"
#include "lib/seq_write.h"
#include "net_commands.h"

buffer SerializeShutdownNetCommand(linear_allocator *Allocator) {
  net_command_type Type = net_command_type_shutdown;
  seq_write S = CreateSeqWrite(Allocator);
  SeqWrite(&S, &Type, sizeof(Type));
  return S.Buffer;
}

net_command_type UnserializeNetCommandType(buffer Buffer) {
  Assert(Buffer.Length >= sizeof(net_command_type));
  return *(net_command_type*)Buffer.Addr;
}

buffer SerializeBroadcastNetCommand(const net_client_id *IDs, memsize IDCount, const buffer Message, linear_allocator *Allocator) {
  seq_write S = CreateSeqWrite(Allocator);
  net_command_type Type = net_command_type_broadcast;
  SeqWrite(&S, &Type, sizeof(Type));
  SeqWriteMemsize(&S, IDCount);
  SeqWriteMemsize(&S, Message.Length);
  SeqWrite(&S, IDs, sizeof(net_client_id)*IDCount);
  SeqWriteBuffer(&S, Message);
  return S.Buffer;
}

buffer SerializeSendNetCommand(net_client_id ID, const buffer Message, linear_allocator *Allocator) {
  seq_write S = CreateSeqWrite(Allocator);
  net_command_type Type = net_command_type_send;
  SeqWrite(&S, &Type, sizeof(Type));
  SeqWrite(&S, &ID, sizeof(ID));
  SeqWriteMemsize(&S, Message.Length);
  SeqWriteBuffer(&S, Message);
  return S.Buffer;
}

broadcast_net_command UnserializeBroadcastNetCommand(buffer Buffer) {
  serializer S = CreateSerializer(Buffer);
  net_command_type Type = *(net_command_type*)SerializerRead(&S, sizeof(net_command_type));
  Assert(Type == net_command_type_broadcast);

  broadcast_net_command Cmd;
  Cmd.ClientIDCount = SerializerReadMemsize(&S);
  Cmd.Message.Length = SerializerReadMemsize(&S);
  Cmd.ClientIDs = (net_client_id*)SerializerRead(&S, sizeof(net_client_id)*Cmd.ClientIDCount);
  Cmd.Message.Addr = SerializerRead(&S, Cmd.Message.Length);

  return Cmd;
}

send_net_command UnserializeSendNetCommand(buffer Buffer) {
  serializer S = CreateSerializer(Buffer);
  net_command_type Type = *(net_command_type*)SerializerRead(&S, sizeof(net_command_type));
  Assert(Type == net_command_type_send);

  send_net_command Cmd;
  Cmd.ClientID = *(net_client_id*)SerializerRead(&S, sizeof(net_client_id));
  Cmd.Message.Length = SerializerReadMemsize(&S);
  Cmd.Message.Addr = SerializerRead(&S, Cmd.Message.Length);

  return Cmd;
}
