#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "orwell.h"

void ReportFailure(const char *Filename, size_t LineNumber, const char *Format, ...) {
  printf("%s:%zu:0: ", Filename, LineNumber);
  va_list Arglist;
  va_start(Arglist, Format);
  vprintf(Format, Arglist);
  va_end(Arglist);
  printf("\n");
}

void _OW_InitGroup(_ow_group *Group) {
  Group->TestCount = 0;
  Group->Tests = NULL;
}

void _OW_TerminateGroup(_ow_group *Group) {
  Group->TestCount = 0;
  free(Group->Tests);
}

void _OW_AssertFalse(ow_test_context Context, const char *Filename, size_t LineNumber, bool Expression) {
  if(Expression != false) {
    ReportFailure(Filename, LineNumber, "Was not false.");
  }
}

void _OW_AssertTrue(ow_test_context Context, const char *Filename, size_t LineNumber, bool Expression) {
  if(Expression != true) {
    ReportFailure(Filename, LineNumber, "Was not true.");
  }
}

void _OW_AssertEqualInt(ow_test_context Context, const char *Filename, size_t LineNumber, int Expected, int Actual) {
  if(Expected != Actual) {
    ReportFailure(Filename, LineNumber, "%d did not equal %d.", Expected, Actual);
  }
}

void _OW_AssertEqualStr(ow_test_context Context, const char *Filename, size_t LineNumber, const char *Expected, const char *Actual) {
  if(strcmp(Expected, Actual) != 0) {
    ReportFailure(Filename, LineNumber, "'%s' did not equal '%s'.", Expected, Actual);
  }
}

void _OW_AssertInDelta(ow_test_context Context, const char *Filename, size_t LineNumber, double Expected, double Delta, double Actual) {
  bool InDelta = (Expected - Delta) < Actual && (Expected + Delta) > Actual;
  if(!InDelta) {
    ReportFailure(Filename, LineNumber, "%f was not within %f +/- %f.", Actual, Expected, Delta);
  }
}

void _OW_AddTest(
  ow_suite *Suite, ow_group_index GroupIndex,
  _ow_test_function Func, const char *Name
) {
  _ow_group *Group = Suite->Groups + GroupIndex;
  size_t NewCapacity = sizeof(_ow_test)*(Group->TestCount + 1);
  Group->Tests = (_ow_test*)realloc(Group->Tests, NewCapacity);
  _ow_test *Test = Group->Tests + Group->TestCount;
  Test->Name = Name;
  Test->Func = Func;
  Group->TestCount++;
}

ow_suite* OW_CreateSuite() {
  ow_suite *Suite = (ow_suite*)malloc(sizeof(ow_suite));
  Suite->GroupCount = 0;
  Suite->Groups = NULL;
  return Suite;
}

void OW_DestroySuite(ow_suite *Suite) {
  for(size_t I=0; I<Suite->GroupCount; ++I) {
    _OW_TerminateGroup(Suite->Groups + I );
  }
  free(Suite->Groups);
  free(Suite);
}

void OW_Run(ow_suite *Suite) {
  for(ow_group_index GroupIndex=0; GroupIndex<Suite->GroupCount; ++GroupIndex) {
    _ow_group *Group = Suite->Groups + GroupIndex;
    for(ow_test_index TestIndex=0; TestIndex<Group->TestCount; ++TestIndex) {
      _ow_test *Test = Group->Tests + TestIndex;
      ow_test_context TestContext;
      TestContext.Suite = Suite;
      TestContext.GroupIndex = GroupIndex;
      TestContext.TestIndex = TestIndex;
      Test->Func(TestContext);
    }
  }
  printf("Orwell done.\n");
}

ow_group_index OW_CreateGroup(ow_suite *Suite) {
  size_t NewCapacity = sizeof(_ow_group)*(Suite->GroupCount+1);
  Suite->Groups = (_ow_group*)realloc(Suite->Groups, NewCapacity);
  _OW_InitGroup(Suite->Groups + Suite->GroupCount);
  return Suite->GroupCount++;
}
