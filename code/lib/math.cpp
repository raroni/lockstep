#include <math.h>
#include "assert.h"
#include "math.h"

int MaxInt(int A, int B) {
  return A > B ? A : B;
}

int MinInt(int A, int B) {
  return A < B ? A : B;
}

int ClampInt(int N, int Min, int Max) {
  N = MaxInt(N, Min);
  N = MinInt(N, Max);
  return N;
}

memsize MaxMemsize(memsize A, memsize B) {
  return A > B ? A : B;
}

memsize MinMemsize(memsize A, memsize B) {
  return A < B ? A : B;
}

r32 MinR32(r32 A, r32 B) {
  return A < B ? A : B;
}

r32 MaxR32(r32 A, r32 B) {
  return A > B ? A : B;
}

r32 SquareRoot(r32 R) {
  return sqrt(R);
}

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

ivec2& operator+=(ivec2 &A, ivec2 B) {
  A = A + B;
  return A;
}

ivec2& operator-=(ivec2 &A, ivec2 B) {
  A = A - B;
  return A;
}

ivec2 operator-(ivec2 A, ivec2 B) {
  ivec2 Result;
  Result.X = A.X - B.X;
  Result.Y = A.Y - B.Y;
  return Result;
}

bool operator==(const ivec2 &A, const ivec2 &B) {
  return A.X == B.X && A.Y == B.Y;
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

rvec2& operator+=(rvec2 &A, rvec2 B) {
  A = A + B;
  return A;
}

rvec2 operator-(rvec2 A, rvec2 B) {
  rvec2 Result;
  Result.X = A.X - B.X;
  Result.Y = A.Y - B.Y;
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

r32 CalcRvec2SquaredMagnitude(rvec2 V) {
  return V.X * V.X + V.Y * V.Y;
}

r32 CalcRvec2Magnitude(rvec2 V) {
  r32 SquaredMag = CalcRvec2SquaredMagnitude(V);
  return SquareRoot(SquaredMag);
}

rvec2 NormalizeRvec2(rvec2 V) {
  r32 Mag = CalcRvec2Magnitude(V);
  Assert(Mag != 0);
  return V / Mag;
}

rvec2 ClampRvec2(rvec2 V, r32 MaxMag) {
  r32 ActualMag = CalcRvec2Magnitude(V);
  if(ActualMag > MaxMag) {
    return (V / ActualMag) * MaxMag;
  }
  else {
    return V;
  }
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

int AbsInt(int X) {
  return abs(X);
}

r32 AbsR32(r32 R) {
  return fabs(R);
}

rrec CreateRrec(rvec2 A, rvec2 B) {
  rrec Result;

  Result.Min.X = MinR32(A.X, B.X);
  Result.Min.Y = MinR32(A.Y, B.Y);

  Result.Max.X = MaxR32(A.X, B.X);
  Result.Max.Y = MaxR32(A.Y, B.Y);

  return Result;
}
