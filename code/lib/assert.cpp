#include "assert.h"

static void HandleFailedAssertion(const char *Filename, size_t Line) {
  fprintf(stderr, "Assert failed: %s:%zu\n", Filename, Line);
  exit(1);
}

void _Assert(bool Expression, const char *Filename, size_t Line) {
  if(!Expression) {
    HandleFailedAssertion(Filename, Line);
  }
}
