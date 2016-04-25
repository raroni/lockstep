#pragma once

#include "lib/def.h"

enum render_command_type {
  render_command_type_draw_square,
  render_command_type_projection
};

struct draw_square_render_command {
  si16 X;
  si16 Y;
  ui32 Color;
};

struct projection_render_command {
  r32 AspectRatio;
  r32 Zoom;
};
