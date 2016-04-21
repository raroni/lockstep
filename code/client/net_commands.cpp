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

memsize SerializeSendNetCommand(buffer Output, buffer Message) {
  net_command_type Type = net_command_type_send;
  serializer S = CreateSerializer(Output);
  SerializerWrite(&S, &Type, sizeof(Type));
  SerializerWrite(&S, Message.Addr, Message.Length);
  return S.Position;
}

send_net_command UnserializeSendNetCommand(buffer Input) {
  serializer S = CreateSerializer(Input);
  net_command_type Type = *(net_command_type*)SerializerRead(&S, sizeof(net_command_type));
  Assert(Type == net_command_type_send);

  memsize MessageLength = GetRemainingSize(&S);
  send_net_command Command;
  Command.Message.Addr = SerializerRead(&S, MessageLength);
  Command.Message.Length = MessageLength;

  return Command;
}
