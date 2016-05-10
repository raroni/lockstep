#pragma once

#include "lib/def.h"
#include "lib/math.h"

enum render_command_type {
  render_command_type_draw_square,
  render_command_type_draw_rect,
  render_command_type_clear_color,
  render_command_type_projection
};

struct draw_square_render_command {
  r32 X;
  r32 Y;
  r32 HalfSize;
  ui32 Color;
};

struct draw_rect_render_command {
  rrect Rect;
  ui32 Color;
};

struct clear_color_render_command {
  ui32 Color;
};

struct projection_render_command {
  r32 AspectRatio;
  r32 Zoom;
};
