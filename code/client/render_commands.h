#pragma once

#include "lib/def.h"

enum render_command_type {
  render_command_type_draw_square
};

struct draw_square_render_command {
  ui16 X;
  ui16 Y;
};
