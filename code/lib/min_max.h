#ifndef MIN_MAX_H
#define MIN_MAX_H

#include "def.h"

static inline int MaxInt(int A, int B) {
  return A > B ? A : B;
}

static inline memsize MaxMemsize(memsize A, memsize B) {
  return A > B ? A : B;
}

static inline memsize MinMemsize(memsize A, memsize B) {
  return A < B ? A : B;
}

#endif
