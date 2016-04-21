#include "lib/chunk_list.h"

void InitGame(buffer Memory);
void UpdateGame(
  bool TerminationRequested,
  chunk_list *NetEvents,
  chunk_list *NetCmds,
  bool *Running,
  buffer Memory
);
