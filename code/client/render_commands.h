#pragma once

#include "lib/def.h"

enum render_command_type {
  render_command_type_draw_square
};

struct draw_square_render_command {
  si16 X;
  si16 Y;
  ui32 Color;
};
