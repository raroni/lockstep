#pragma once

#include "lib/def.h"

enum render_command_type {
  render_command_type_draw_square,
  render_command_type_clear_color,
  render_command_type_projection
};

struct draw_square_render_command {
  si16 X;
  si16 Y;
  ui8 HalfSize;
  ui32 Color;
};

struct clear_color_render_command {
  ui32 Color;
};

struct projection_render_command {
  r32 AspectRatio;
  r32 Zoom;
};
