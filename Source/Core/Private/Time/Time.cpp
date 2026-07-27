#include "OptimEngine/Core/Time/Time.h"
#include "SystemTime.h"

Timer::Timer() {
}

void Timer::Start() {
  value = OptimPrivate::Time::GetTime();
}

void Timer::End() {
  uint64 endVal = OptimPrivate::Time::GetTime();
  value = endVal - value;
}

uint64 Timer::Get() {
  return value;
}