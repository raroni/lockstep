#include "lib/chunk_list.h"

void InitGame(buffer Memory);
void UpdateGame(
  uusec64 Time,
  uusec64 *Delay,
  bool TerminationRequested,
  chunk_list *NetEvents,
  chunk_list *NetCmds,
  bool *Running,
  buffer Memory
);
