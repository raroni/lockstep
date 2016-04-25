#include "orwell.h"

void SetupByteRingBufferGroup(ow_suite *Suite);
void SetupChunkRingBufferGroup(ow_suite *Suite);
void SetupPosixNetClientSetIteratorGroup(ow_suite *Suite);
void SetupChunkListGroup(ow_suite *Suite);
void SetupIntSeqGroup(ow_suite *Suite);
void SetupMathGroup(ow_suite *Suite);

int main() {
  ow_suite *Suite = OW_CreateSuite();

  SetupByteRingBufferGroup(Suite);
  SetupChunkRingBufferGroup(Suite);
  SetupPosixNetClientSetIteratorGroup(Suite);
  SetupChunkListGroup(Suite);
  SetupIntSeqGroup(Suite);
  SetupMathGroup(Suite);

  OW_Run(Suite);
  OW_DestroySuite(Suite);

  return 0;
}
