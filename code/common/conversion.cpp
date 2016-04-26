#include "lib/assert.h"
#include "conversion.h"

ui8 SafeCastIntToUI8(int Value) {
  Assert(Value <= UINT8_MAX);
  ui8 Result = (ui8)Value;
  return(Result);
}

ui16 SafeCastIntToUI16(int Value) {
  Assert(Value <= UINT16_MAX);
  ui16 Result = (ui16)Value;
  return(Result);
}

si16 SafeCastIntToSI16(int Value) {
  Assert(Value <= INT16_MAX);
  si16 Result = (si16)Value;
  return(Result);
}
