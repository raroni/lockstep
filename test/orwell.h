#ifndef ORWELL_H
#define ORWELL_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t ow_group_index;
typedef uint16_t ow_test_index;

struct ow_test_context {
  struct ow_suite *Suite;
  ow_group_index GroupIndex;
  ow_test_index TestIndex;
};

typedef void (*_ow_test_function)(ow_test_context Context);

struct _ow_test {
  const char *Name;
  _ow_test_function Func;
};

struct _ow_group {
  size_t TestCount;
  _ow_test *Tests;
};

struct ow_suite {
  size_t GroupCount;
  _ow_group *Groups;
};

#define OW_AssertFalse(Expression) _OW_AssertFalse(Context, __FILE__, __LINE__, Expression)
void _OW_AssertFalse(ow_test_context Context, const char *Filename, size_t LineNumber, bool Expression);

#define OW_AssertTrue(Expression) _OW_AssertTrue(Context, __FILE__, __LINE__, Expression)
void _OW_AssertTrue(ow_test_context Context, const char *Filename, size_t LineNumber, bool Expression);

#define OW_AssertEqualInt(Expected, Actual) _OW_AssertEqualInt(Context, __FILE__, __LINE__, Expected, Actual)
void _OW_AssertEqualInt(ow_test_context Context, const char *Filename, size_t LineNumber, int Expected, int Actual);

#define OW_AssertEqualStr(Expected, Actual) _OW_AssertEqualStr(Context, __FILE__, __LINE__, Expected, Actual)
void _OW_AssertEqualStr(ow_test_context Context, const char *Filename, size_t LineNumber, const char *Expected, const char *Actual);

#define OW_AssertInDelta(Expected, Delta, Actual) _OW_AssertInDelta(Context, __FILE__, __LINE__, Expected, Delta, Actual)
void _OW_AssertInDelta(ow_test_context Context, const char *Filename, size_t LineNumber, double Expected, double Delta, double Actual);

#define OW_AddTest(Suite, GroupIndex, Name) _OW_AddTest(Suite, GroupIndex, Name, #Name)
void _OW_AddTest(
  ow_suite *Suite, ow_group_index GroupIndex,
  _ow_test_function Func, const char *Name
);

ow_suite* OW_CreateSuite();

void OW_DestroySuite(ow_suite *Suite);
void OW_Run(ow_suite *Suite);
ow_group_index OW_CreateGroup(ow_suite *Suite);

#endif
