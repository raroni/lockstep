#include "assert.h"

void _Assert(bool Expression, const char *Filename, size_t Line) {
  if(!Expression) {
    fprintf(stderr, "Assert failed: %s:%zu\n", Filename, Line);
    exit(1);
  }
}
