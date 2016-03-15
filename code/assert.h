#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>
#include <stdlib.h>

void _Assert(bool Expression, const char *Filename, size_t Line) {
  if(!Expression) {
    fprintf(stderr, "Assert failed: %s:%zu\n", Filename, Line);
    exit(1);
  }
}

#define Assert(Expression) _Assert(Expression, __FILE__, __LINE__)
#define InvalidCodePath Assert(false)

#endif
