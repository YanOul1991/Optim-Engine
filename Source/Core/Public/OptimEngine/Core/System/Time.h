#pragma once

#include "Core/CoreMinimal.h"

namespace Time {

constexpr double nanoPerMilli = 1000000;
constexpr double nanoPerMicro = 1000;

/**
 * Returns the time since startup of the computer in nanoseconds.
 */
CORE_API uint64 GetTime();

}; // namespace System::Time
