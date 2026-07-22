#pragma once

#include "Core/Minimal/Concepts.h"
#include "Core/Minimal/Intrinsics.h"
#include "Core/Minimal/Macros.h"

// Generated Files
#include "Core/Minimal/Generated/CoreModuleGenerated.h"
#include "Core/Minimal/Generated/RenderModuleGenerated.h"
#include "Core/Minimal/Generated/RhiModuleGenerated.h"
#include "Core/Minimal/Generated/SystemModuleGenerated.h"
#include "Core/Minimal/Generated/VulkanRhiModuleGenerated.h"

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