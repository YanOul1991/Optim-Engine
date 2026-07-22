// Logging.cpp

#include "Core/Debug/Logging.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cstdarg>

void Internal::Debug::InternalDebugPrintF(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stdout, format, args);
  va_end(args);
}
