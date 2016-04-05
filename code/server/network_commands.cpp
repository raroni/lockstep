#include "lib/assert.h"
#include "common/serialization.h"
#include "network_commands.h"

memsize SerializeDisconnectNetworkCommand(void *Buffer, memsize Capacity) {
  network_command_type Type = network_command_type_disconnect;
  serializer S = CreateSerializer(Buffer, Capacity);
  SerializerWrite(&S, &Type, sizeof(Type));
  return sizeof(Type);
}

network_command_type UnserializeNetworkCommandType(void *Buffer, memsize Capacity) {
  Assert(Capacity >= sizeof(network_command_type));
  return *(network_command_type*)Buffer;
}

memsize SerializeBroadcastNetworkCommand(
    const client_id *IDs,
    memsize IDCount,
    const void *Message,
    memsize MessageLength,
    void *Buffer,
    memsize BufferCapacity
) {
  serializer S = CreateSerializer(Buffer, BufferCapacity);
  network_command_type Type = network_command_type_broadcast;
  SerializerWrite(&S, &Type, sizeof(Type));
  SerializerWriteMemsize(&S, IDCount);
  SerializerWriteMemsize(&S, MessageLength);
  SerializerWrite(&S, IDs, sizeof(client_id)*IDCount);
  SerializerWrite(&S, Message, MessageLength);
  return S.Position;
}

broadcast_network_command UnserializeBroadcastNetworkCommand(void *Buffer, memsize Capacity) {
  serializer S = CreateSerializer(Buffer, Capacity);
  network_command_type Type = *(network_command_type*)SerializerRead(&S, sizeof(network_command_type));
  Assert(Type == network_command_type_broadcast);

  broadcast_network_command Cmd;
  Cmd.ClientIDCount = SerializerReadMemsize(&S);
  Cmd.MessageLength = SerializerReadMemsize(&S);
  Cmd.ClientIDs = (client_id*)SerializerRead(&S, sizeof(client_id)*Cmd.ClientIDCount);
  Cmd.Message = SerializerRead(&S, Cmd.MessageLength);

  return Cmd;
}
