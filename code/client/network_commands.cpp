#include "lib/assert.h"
#include "lib/serialization.h"
#include "network_commands.h"

memsize SerializeShutdownNetworkCommand(buffer Buffer) {
  network_command_type Type = network_command_type_shutdown;
  serializer S = CreateSerializer(Buffer);
  SerializerWrite(&S, &Type, sizeof(Type));
  return sizeof(Type);
}

network_command_type UnserializeNetworkCommandType(buffer Buffer) {
  Assert(Buffer.Length >= sizeof(network_command_type));
  return *(network_command_type*)Buffer.Addr;
}

memsize SerializeSendNetworkCommand(buffer Output, buffer Message) {
  network_command_type Type = network_command_type_send;
  serializer S = CreateSerializer(Output);
  SerializerWrite(&S, &Type, sizeof(Type));
  SerializerWrite(&S, Message.Addr, Message.Length);
  return S.Position;
}

send_network_command UnserializeSendNetworkCommand(buffer Input) {
  serializer S = CreateSerializer(Input);
  network_command_type Type = *(network_command_type*)SerializerRead(&S, sizeof(network_command_type));
  Assert(Type == network_command_type_send);

  memsize MessageLength = GetRemainingSize(&S);
  send_network_command Command;
  Command.Message.Addr = SerializerRead(&S, MessageLength);
  Command.Message.Length = MessageLength;

  return Command;
}
