#pragma once

#include "lib/math.h"
#include "lib/chunk_list.h"

struct game_mouse {
  ivec2 Pos;
  bool ButtonPressed;
  ui8 ButtonChangeCount;
};

struct game_platform {
  uusec64 Time;
  bool TerminationRequested;
  game_mouse *Mouse;
  ivec2 Resolution;
};

void InitGame(buffer Memory);
void UpdateGame(
  game_platform *Platform,
  chunk_list *NetEvents,
  chunk_list *NetCmds,
  chunk_list *RenderCmds,
  bool *Running,
  buffer Memory
);
