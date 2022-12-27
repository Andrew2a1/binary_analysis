#pragma once

#include <pin.H>

#define TO_AFUNPTR(f) (reinterpret_cast<AFUNPTR>(reinterpret_cast<uint64_t>(f)))
