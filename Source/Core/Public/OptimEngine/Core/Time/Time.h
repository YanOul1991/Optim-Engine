#pragma once

#include "OptimEngine/Core/CoreMinimal.h"

namespace Optim::Time {
// Number of nanoseconds in 1 millisecond.
constexpr double nanosecPerMillisec = 1'000'000.0;
// Number of nanoseconds in 1 microsecond.
constexpr double nanosecPerMicrosec = 1'000.0;
}; // namespace Optim::Time

class CORE_API Timer final
{
 public:
  NOCOPY(Timer)
  NOMOVE(Timer)

  Timer();

  /**
   * Starts the Timer.
   */
  void Start();

  /**
   * Ends the Timer.
   */
  void End();

  /**
   * Get the timer value in nanoseconds.
   */
  uint64 Get();

 private:
  uint64 value;
};