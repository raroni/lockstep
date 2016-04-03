#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>
#include <stdlib.h>

void _Assert(bool Expression, const char *Filename, size_t Line);
#define Assert(Expression) _Assert(Expression, __FILE__, __LINE__)
#define InvalidCodePath Assert(false)

#endif
