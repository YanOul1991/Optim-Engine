#pragma once

#include "Core/Minimal/Concepts.h"
#include "Core/Minimal/Intrinsics.h"
#include "Core/Minimal/Macros.h"
#include "Core/Minimal/ModuleExportMacros.h"

#if (__cplusplus == 202603l)
#  define CPP_VER 26
#elif (__cplusplus == 202302l)
#  define CPP_VER 23
#elif (__cplusplus == 202002l)
#  define CPP_VER 20
#elif (__cplusplus == 201703l)
#  define CPP_VER 17
#elif (__cplusplus == 201402l)
#  define CPP_VER 14
#elif (__cplusplus == 201103l)
#  define CPP_VER 11
#elif (__cplusplus == 199711l)
#  define CPP_VER 98
#else
# error "Unknown C++ Version"
#endif

consteval const char* const GetCPPVersion()
{
  if (__cplusplus == 202603l) {
    return "C++26\n";
  }
  else if (__cplusplus == 202302l) {
    return "C++23\n";
  }
  else if (__cplusplus == 202002l) {
    return "C++20\n";
  }
  else if (__cplusplus == 201703l) {
    return "C++17\n";
  }
  else if (__cplusplus == 201402l) {
    return "C++14\n";
  }
  else if (__cplusplus == 201103l) {
    return "C++11\n";
  }
  else if (__cplusplus == 199711l) {
    return "C++98\n";
  }
  else {
    return "unknown C++ version\n";
  }
}