#include "lib/assert.h"
#include "conversion.h"

ui8 SafeCastIntToUI8(int Value) {
    Assert(Value <= UINT8_MAX);
    ui8 Result = (ui8)Value;
    return(Result);
}
