#include "orwell.h"
#include "server/client_set.h"

typedef client_set set;
typedef client_set_iterator iterator;

static void TestBasic(ow_test_context Context) {
  set Set;
  InitClientSet(&Set);
  CreateClient(&Set, 12);
  CreateClient(&Set, 34);

  iterator Iterator = CreateClientSetIterator(&Set);
  OW_AssertTrue(AdvanceClientSetIterator(&Iterator));
  OW_AssertEqualInt(12, Iterator.Client->FD);
  OW_AssertTrue(AdvanceClientSetIterator(&Iterator));
  OW_AssertEqualInt(34, Iterator.Client->FD);
  OW_AssertFalse(AdvanceClientSetIterator(&Iterator));

  TerminateClientSet(&Set);
}

static void TestDestruction(ow_test_context Context) {
  set Set;
  InitClientSet(&Set);
  CreateClient(&Set, 1);
  CreateClient(&Set, 2);
  CreateClient(&Set, 3);

  iterator Iterator = CreateClientSetIterator(&Set);
  AdvanceClientSetIterator(&Iterator);
  AdvanceClientSetIterator(&Iterator);
  DestroyClient(&Iterator);
  AdvanceClientSetIterator(&Iterator);
  OW_AssertEqualInt(3, Iterator.Client->FD);
  AdvanceClientSetIterator(&Iterator);
  OW_AssertFalse(AdvanceClientSetIterator(&Iterator));

  TerminateClientSet(&Set);
}

void SetupClientSetIteratorGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasic);
  OW_AddTest(S, G, TestDestruction);
}
