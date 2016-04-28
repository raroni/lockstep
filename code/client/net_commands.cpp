#include "lib/assert.h"
#include "lib/buf_view.h"
#include "lib/seq_write.h"
#include "net_commands.h"

buffer SerializeShutdownNetCommand(memory_arena *Arena) {
  net_command_type Type = net_command_type_shutdown;
  seq_write S = CreateSeqWrite(Arena);
  SeqWrite(&S, &Type, sizeof(Type));
  return S.Buffer;
}

net_command_type UnserializeNetCommandType(buffer Buffer) {
  Assert(Buffer.Length >= sizeof(net_command_type));
  return *(net_command_type*)Buffer.Addr;
}

buffer SerializeSendNetCommand(buffer Message, memory_arena *Arena) {
  net_command_type Type = net_command_type_send;
  seq_write S = CreateSeqWrite(Arena);
  SeqWrite(&S, &Type, sizeof(Type));
  SeqWriteBuffer(&S, Message);
  return S.Buffer;
}

send_net_command UnserializeSendNetCommand(buffer Input) {
  buf_view V = CreateBufView(Input);
  net_command_type Type = *(net_command_type*)BufViewRead(&V, sizeof(net_command_type));
  Assert(Type == net_command_type_send);

  memsize MessageLength = GetRemainingSize(&V);
  send_net_command Command;
  Command.Message.Addr = BufViewRead(&V, MessageLength);
  Command.Message.Length = MessageLength;

  return Command;
}
