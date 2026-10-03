#pragma once

#include "Core/CoreMinimal.h"

class CORE_API Timer final
{
 public:
  NO_COPY(Timer)
  NO_MOVE(Timer)

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
  constexpr inline uint64 Get() const {
    return value;
  }

  constexpr inline double GetMs() const {
    return static_cast<double>(value) / static_cast<double>(1'000'000);
  }

 private:
  uint64 value;
};