#include "lib/chunk_list.h"

void InitServer(buffer Memory);
void UpdateServer(
  bool TerminationRequested,
  chunk_list *NetEvents,
  chunk_list *NetCmds,
  bool *Running,
  buffer Memory
);
