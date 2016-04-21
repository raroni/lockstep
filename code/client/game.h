#pragma once

#include "lib/chunk_list.h"

void InitGame(buffer Memory);
void UpdateGame(
  bool TerminationRequested,
  chunk_list *NetEvents,
  chunk_list *NetCmds,
  chunk_list *RenderCmds,
  bool *Running,
  buffer Memory
);
