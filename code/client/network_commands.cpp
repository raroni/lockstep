#include "lib/assert.h"
#include "common/serialization.h"
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
