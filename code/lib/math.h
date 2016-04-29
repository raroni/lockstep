#pragma once

#include "lib/def.h"

struct ivec2 {
  si16 X;
  si16 Y;
};

ivec2 MakeIvec2(ui16 X, ui16 Y);
ivec2 operator+(ivec2 A, ivec2 B);
ivec2& operator+=(ivec2 &A, ivec2 B);
ivec2 operator-(ivec2 A, ivec2 B);
ivec2& operator-=(ivec2 &A, ivec2 B);
bool operator==(const ivec2 &A, const ivec2 &B);

struct rvec2 {
  r32 X;
  r32 Y;
};

rvec2 MakeRvec2(r32 X, r32 Y);
rvec2 operator+(rvec2 A, rvec2 B);
rvec2& operator+=(rvec2 &A, rvec2 B);
rvec2 operator-(rvec2 A, rvec2 B);
rvec2 operator-(rvec2 V, r32 S);
rvec2 operator*(rvec2 A, r32 S);
rvec2 operator/(rvec2 A, rvec2 B);
rvec2 operator/(rvec2 A, r32 S);

rvec2 ClampRvec2(rvec2 V, r32 Magnitude);
r32 CalcRvec2Magnitude(rvec2 V);
r32 CalcRvec2SquaredMagnitude(rvec2 V);

rvec2 ConvertIvec2ToRvec2(ivec2 V);
ivec2 ConvertRvec2ToIvec2(rvec2 V);

r32 SquareRoot(r32 R);
