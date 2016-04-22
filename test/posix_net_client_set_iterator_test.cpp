#include "orwell.h"
#include "server/posix_net_client_set.h"

typedef posix_net_client_set set;
typedef posix_net_client_set_iterator iterator;

static void TestBasic(ow_test_context Context) {
  set Set;
  InitPosixNetClientSet(&Set);
  CreateClient(&Set, 12);
  CreateClient(&Set, 34);

  iterator Iterator = CreatePosixNetClientSetIterator(&Set);
  OW_AssertTrue(AdvancePosixNetClientSetIterator(&Iterator));
  OW_AssertEqualInt(12, Iterator.Client->FD);
  OW_AssertTrue(AdvancePosixNetClientSetIterator(&Iterator));
  OW_AssertEqualInt(34, Iterator.Client->FD);
  OW_AssertFalse(AdvancePosixNetClientSetIterator(&Iterator));

  TerminatePosixNetClientSet(&Set);
}

static void TestDestruction(ow_test_context Context) {
  set Set;
  InitPosixNetClientSet(&Set);
  CreateClient(&Set, 1);
  CreateClient(&Set, 2);
  CreateClient(&Set, 3);

  iterator Iterator = CreatePosixNetClientSetIterator(&Set);
  AdvancePosixNetClientSetIterator(&Iterator);
  AdvancePosixNetClientSetIterator(&Iterator);
  DestroyClient(&Iterator);
  AdvancePosixNetClientSetIterator(&Iterator);
  OW_AssertEqualInt(3, Iterator.Client->FD);
  AdvancePosixNetClientSetIterator(&Iterator);
  OW_AssertFalse(AdvancePosixNetClientSetIterator(&Iterator));

  TerminatePosixNetClientSet(&Set);
}

void SetupPosixNetClientSetIteratorGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasic);
  OW_AddTest(S, G, TestDestruction);
}
