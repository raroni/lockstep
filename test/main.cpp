#include "orwell.h"

void SetupRingBufferGroup(ow_suite *Suite);

int main() {
  ow_suite *Suite = OW_CreateSuite();

  SetupRingBufferGroup(Suite);

  OW_Run(Suite);
  OW_DestroySuite(Suite);

  return 0;
}
