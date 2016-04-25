#include "lib/math.h"

#include <stdio.h>

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
