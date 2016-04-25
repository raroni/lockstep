#include <math.h>
#include "math.h"

ivec2 MakeIvec2(ui16 X, ui16 Y) {
  ivec2 Result;
  Result.X = X;
  Result.Y = Y;
  return Result;
}

ivec2 operator+(ivec2 A, ivec2 B) {
  ivec2 Result;
  Result.X = A.X + B.X;
  Result.Y = A.Y + B.Y;
  return Result;
}

rvec2 MakeRvec2(r32 X, r32 Y) {
  rvec2 Result;
  Result.X = X;
  Result.Y = Y;
  return Result;
}

rvec2 operator+(rvec2 A, rvec2 B) {
  rvec2 Result;
  Result.X = A.X + B.X;
  Result.Y = A.Y + B.Y;
  return Result;
}

rvec2 operator+(rvec2 V, r32 S) {
  rvec2 Result;
  Result.X = V.X + S;
  Result.Y = V.Y + S;
  return Result;
}

rvec2 operator-(rvec2 V, r32 S) {
  rvec2 Result;
  Result.X = V.X - S;
  Result.Y = V.Y - S;
  return Result;
}

rvec2 operator*(rvec2 V, r32 S) {
  rvec2 Result;
  Result.X = V.X * S;
  Result.Y = V.Y * S;
  return Result;
}

rvec2 operator/(rvec2 A, rvec2 B) {
  rvec2 Result;
  Result.X = A.X / B.X;
  Result.Y = A.Y / B.Y;
  return Result;
}

rvec2 operator/(rvec2 V, r32 S) {
  rvec2 Result;
  Result.X = V.X / S;
  Result.Y = V.Y / S;
  return Result;
}

rvec2 ConvertIvec2ToRvec2(ivec2 V) {
  rvec2 Result;
  Result.X = V.X;
  Result.Y = V.Y;
  return Result;
}

ivec2 ConvertRvec2ToIvec2(rvec2 V) {
  ivec2 Result;
  Result.X = round(V.X);
  Result.Y = round(V.Y);
  return Result;
}
