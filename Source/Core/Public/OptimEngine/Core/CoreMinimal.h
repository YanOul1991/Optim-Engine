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