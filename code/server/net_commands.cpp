#include "lib/assert.h"
#include "lib/serialization.h"
#include "net_commands.h"

memsize SerializeShutdownNetCommand(buffer Buffer) {
  net_command_type Type = net_command_type_shutdown;
  serializer S = CreateSerializer(Buffer);
  SerializerWrite(&S, &Type, sizeof(Type));
  return sizeof(Type);
}

net_command_type UnserializeNetCommandType(buffer Buffer) {
  Assert(Buffer.Length >= sizeof(net_command_type));
  return *(net_command_type*)Buffer.Addr;
}

memsize SerializeBroadcastNetCommand(const net_client_id *IDs, memsize IDCount, const buffer Message, buffer Out) {
  serializer S = CreateSerializer(Out);
  net_command_type Type = net_command_type_broadcast;
  SerializerWrite(&S, &Type, sizeof(Type));
  SerializerWriteMemsize(&S, IDCount);
  SerializerWriteMemsize(&S, Message.Length);
  SerializerWrite(&S, IDs, sizeof(net_client_id)*IDCount);
  SerializerWriteBuffer(&S, Message);
  return S.Position;
}

memsize SerializeSendNetCommand(net_client_id ID, const buffer Message, buffer Out) {
  serializer S = CreateSerializer(Out);
  net_command_type Type = net_command_type_send;
  SerializerWrite(&S, &Type, sizeof(Type));
  SerializerWrite(&S, &ID, sizeof(ID));
  SerializerWriteMemsize(&S, Message.Length);
  SerializerWriteBuffer(&S, Message);
  return S.Position;
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
