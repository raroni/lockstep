#include "orwell.h"
#include "lib/math.h"

static void TestIvec2Addition(ow_test_context Context) {
  ivec2 A = MakeIvec2(2, 4);
  ivec2 B = MakeIvec2(8, 2);
  ivec2 C = A + B;
  OW_AssertEqualInt(10, C.X);
  OW_AssertEqualInt(6, C.Y);
}

static void TestRvec2Addition(ow_test_context Context) {
  rvec2 A = MakeRvec2(2, 4);
  rvec2 B = MakeRvec2(8, 2);
  rvec2 C = A + B;
  OW_AssertInDelta(10, 0.0001, C.X);
  OW_AssertInDelta(6, 0.0001, C.Y);
}

void SetupMathGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestIvec2Addition);
  OW_AddTest(S, G, TestRvec2Addition);
}
