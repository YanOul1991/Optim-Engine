#pragma once

#if defined(_MSC_VER)
using int8   = __int8;
using int16  = __int16;
using int32  = __int32;
using int64  = __int64;
using uint8  = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;
using byte   = unsigned __int8;
#elif defined(__GNUC__)
using int8   = __INT8_TYPE__;
using int16  = __INT16_TYPE__;
using int32  = __INT32_TYPE__;
using int64  = __INT64_TYPE__;
using uint8  = __UINT8_TYPE__;
using uint16 = __UINT16_TYPE__;
using uint32 = __UINT32_TYPE__;
using uint64 = __UINT64_TYPE__;
using byte   = __UINT8_TYPE__;
#endif

#define NOCOPY(CLASS_NAME)                           \
  CLASS_NAME(const CLASS_NAME&)            = delete; \
  CLASS_NAME& operator=(const CLASS_NAME&) = delete;

#define NOMOVE(CLASS_NAME)                      \
  CLASS_NAME(CLASS_NAME&&)            = delete; \
  CLASS_NAME& operator=(CLASS_NAME&&) = delete;