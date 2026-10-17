#pragma once

#include <fmt/base.h>
#include <utility>

namespace Debug
{

template <typename... ARGS> 
inline void PrintLn(fmt::format_string<ARGS...> fmt, ARGS&&... args)
{
#if !defined(NDEBUG)
  fmt::println(stdout, fmt, std::forward<ARGS&&>(args)...);
#endif
}

template <typename... ARGS> 
inline void Print(fmt::format_string<ARGS...> fmt, ARGS&&... args)
{
#if !defined(NDEBUG)
  fmt::print(stdout, fmt, std::forward<ARGS&&>(args)...);
#endif
}

template <typename... ARGS> 
inline void PrintError(fmt::format_string<ARGS...> fmt, ARGS&&... args)
{
#if !defined(NDEBUG)
  fmt::print(stderr, fmt, std::forward<ARGS&&>(args)...);
#endif
}

}; // namespace Debug
