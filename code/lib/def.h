#ifndef DEF_H
#define DEF_H

#include <stddef.h>
#include <stdint.h>

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

typedef size_t memsize;

struct buffer {
  void *Addr;
  memsize Length;
};

#endif
