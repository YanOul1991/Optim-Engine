#pragma once

#include "Core/CoreMinimal.h"

namespace LowLevel {

#if defined(__GNUC__)
#  define SIMD(TARGET) __attribute__((target(TARGET)))
#else
#  define SIMD(TARGET)
#endif

SIMD("avx2")
int32 CORE_API FindSimdAvx2(int32 target, int32* pBlock);

}; // namespace System::LowLevel