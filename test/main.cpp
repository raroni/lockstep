#include "orwell.h"

void SetupByteRingBufferGroup(ow_suite *Suite);

int main() {
  ow_suite *Suite = OW_CreateSuite();

  SetupByteRingBufferGroup(Suite);

  OW_Run(Suite);
  OW_DestroySuite(Suite);

  return 0;
}
