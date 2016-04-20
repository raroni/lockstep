#pragma once

#include "lib/chunk_list.h"

void InitClient(buffer Memory);
void UpdateClient(
  bool TerminationRequested,
  chunk_list *NetEvents,
  chunk_list *NetCmds,
  bool *Running,
  buffer Memory
);
