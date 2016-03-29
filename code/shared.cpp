#include "shared.h"

ui8 SafeCastIntToUI8(int Value) {
    // TODO(casey): Defines for maximum values
    Assert(Value <= INT_MAX);
    ui8 Result = (ui8)Value;
    return(Result);
}
