#pragma once

#include "Core/Minimal/Concepts.h"
#include "Core/Minimal/Intrinsics.h"
#include "Core/Minimal/Macros.h"
#include "Core/Minimal/ModuleExportMacros.h"

// Generated Files
// #include "OptimEngine/Core/Minimal/Generated/core_exports_generated.h"
// #include "OptimEngine/Core/Minimal/Generated/rendering_exports_generated.h"
// #include "OptimEngine/Core/Minimal/Generated/rhi_exports_generated.h"
// #include "OptimEngine/Core/Minimal/Generated/system_exports_generated.h"
// #include "OptimEngine/Core/Minimal/Generated/vulkanrhi_exports_generated.h"

consteval const char* const GetCPPVersion() {
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