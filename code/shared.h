#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>
#include <limits.h>
#include "assert.h"

typedef float real32;
typedef double real64;

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;
typedef int8_t si8;
typedef int16_t si16;
typedef int32_t si32;
typedef int64_t si64;

typedef ui16 umsec16;
typedef si16 smsec16;
typedef ui32 umsec32;
typedef si32 smsec32;
typedef ui64 smsec64;
typedef ui64 uusec64;

enum errno_code {
  errno_code_interrupted_system_call = 4,
  errno_code_in_progress = 36
};

static inline int MaxInt(int A, int B) {
  return A > B ? A : B;
}

ui8 SafeCastIntToUI8(int Value);

#endif
