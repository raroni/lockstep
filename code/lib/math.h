#pragma once

#include "lib/def.h"

int MaxInt(int A, int B);
int MinInt(int A, int B);
int ClampInt(int N, int Min, int Max);
memsize MaxMemsize(memsize A, memsize B);
memsize MinMemsize(memsize A, memsize B);
r32 MinR32(r32 A, r32 B);
r32 MaxR32(r32 A, r32 B);

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

struct rrec {
  rvec2 Min;
  rvec2 Max;
};

rrec CreateRrec(rvec2 A, rvec2 B);

r32 SquareRoot(r32 R);
int AbsInt(int N);
r32 AbsR32(r32 R);
