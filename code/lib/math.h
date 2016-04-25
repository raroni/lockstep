#pragma once

#include "lib/def.h"

struct ivec2 {
  ui16 X;
  ui16 Y;
};

ivec2 MakeIvec2(ui16 X, ui16 Y);
ivec2 operator+(const ivec2 A, const ivec2 B);

struct rvec2 {
  r32 X;
  r32 Y;
};

rvec2 MakeRvec2(r32 X, r32 Y);
rvec2 operator+(const rvec2 A, const rvec2 B);
rvec2 ConvertIvec2ToRvec2(ivec2 V);
