#pragma once

#include "lib/chunk_list.h"

struct game_mouse {
  ui16 PosX;
  ui16 PosY;
  bool ButtonPressed;
  ui8 ButtonChangeCount;
};

void InitGame(buffer Memory);
void UpdateGame(
  uusec64 Time,
  bool TerminationRequested,
  game_mouse *Mouse,
  chunk_list *NetEvents,
  chunk_list *NetCmds,
  chunk_list *RenderCmds,
  bool *Running,
  buffer Memory
);
