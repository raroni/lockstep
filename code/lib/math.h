#include "lib/def.h"

struct ivec2 {
  ui16 X;
  ui16 Y;
};

ivec2 MakeIvec2(ui16 X, ui16 Y) {
  ivec2 Result;
  Result.X = X;
  Result.Y = Y;
  return Result;
}

ivec2 operator+(const ivec2 A, const ivec2 B) {
  ivec2 Result;
  Result.X = A.X + B.X;
  Result.Y = A.Y + B.Y;
  return Result;
}

struct rvec2 {
  r32 X;
  r32 Y;
};

rvec2 MakeRvec2(r32 X, r32 Y) {
  rvec2 Result;
  Result.X = X;
  Result.Y = Y;
  return Result;
}

rvec2 operator+(const rvec2 A, const rvec2 B) {
  rvec2 Result;
  Result.X = A.X + B.X;
  Result.Y = A.Y + B.Y;
  return Result;
}

rvec2 ConvertIvec2ToRvec2(ivec2 V) {
  rvec2 Result;
  Result.X = V.X;
  Result.Y = V.Y;
  return Result;
}
