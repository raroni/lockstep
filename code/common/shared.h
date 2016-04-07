#pragma once

#include <stdint.h>
#include <limits.h>
#include "assert.h"
#include "lib/def.h"

enum errno_code {
  errno_code_interrupted_system_call = 4,
  errno_code_in_progress = 36
};

ui8 SafeCastIntToUI8(int Value);
