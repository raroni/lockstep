#include "lib/math.h"

rvec2 ConvertWindowToNDCCoors(ivec2 WindowCoors, ivec2 Resolution) {
  rvec2 Result;

  rvec2 RealWindowCoors = ConvertIvec2ToRvec2(WindowCoors);
  rvec2 RealResolution = ConvertIvec2ToRvec2(Resolution);

  Result = (RealWindowCoors / RealResolution) * 2 - 1;

  return Result;
}

ivec2 ConvertNDCToWorldCoors(rvec2 NDCPos, r32 AspectRatio, r32 Zoom) {
  rvec2 Temp = NDCPos / Zoom;
  Temp.Y /= AspectRatio;
  ivec2 Result = ConvertRvec2ToIvec2(Temp);
  return Result;
}

ivec2 ConvertWindowToWorldCoors(ivec2 WindowCoors, ivec2 Resolution, r32 AspectRatio, r32 Zoom) {
  rvec2 NDCCoors = ConvertWindowToNDCCoors(WindowCoors, Resolution);
  ivec2 Result = ConvertNDCToWorldCoors(NDCCoors, AspectRatio, Zoom);
  return Result;
}

rvec2 ConvertNDCToUICoors(rvec2 WindowCoors, r32 AspectRatio) {
  rvec2 Result;
  Result.X = WindowCoors.X;
  Result.Y = WindowCoors.Y / AspectRatio;
  return Result;
}

rvec2 ConvertWindowToUICoors(ivec2 WindowCoors, ivec2 Resolution, r32 AspectRatio) {
  rvec2 NDCCoors = ConvertWindowToNDCCoors(WindowCoors, Resolution);
  rvec2 Result = ConvertNDCToUICoors(NDCCoors, AspectRatio);
  return Result;
}

rvec2 ConvertUIToNDCCoors(rvec2 UIPos, r32 AspectRatio) {
  rvec2 Result;
  Result.X = UIPos.X;
  Result.Y = UIPos.Y * AspectRatio;
  return Result;
}

ivec2 ConvertUIToWorldCoors(rvec2 UIPos, r32 AspectRatio, r32 Zoom) {
  rvec2 NDCPos = ConvertUIToNDCCoors(UIPos, AspectRatio);
  ivec2 Result = ConvertNDCToWorldCoors(NDCPos, AspectRatio, Zoom);
  return Result;
}
