// Debug.cpp

#include "Core/Debug/Debug.h"
// For excecutable callstack backtracking

#if !defined(PLATFORM_WINDOWS)
#  include <execinfo.h>
// Namedemagling
#  include <cxxabi.h>
#  include <dlfcn.h> // You may need to link with -ldl
#  include <link.h>
#endif
// std
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct SCallStack final {
  std::string location;
  std::string name;
  // std::string strAddress;
  uintptr_t address;
};

static std::vector<SCallStack> GetTranslatedCallStacks(void** _param_list, char** _param_symbols, const size_t _param_count) {
  std::vector<SCallStack> __prettyCallStacks;
#if !defined(PLATFORM_WINDOWS)
  std::string __strLocation;
  std::string __strName;
  std::string __address;

  for (size_t i = 0; i < _param_count; i++) {
    std::string ___symbol(_param_symbols[i]);

    size_t ___firstBracket = ___symbol.find_first_of('(');
    size_t ___endOfName    = ___symbol.find_last_of('+');
    // size_t ___addressStart = ___symbol.find_last_of('[');
    // size_t ___addressEnd   = ___symbol.find_last_of(']');

    // If no name can be found then the call cannot
    // be debugged and skip to next symbol
    if (___firstBracket == std::string::npos || ___endOfName == std::string::npos) {
      continue;
    }

    SCallStack ___retVal;

    size_t      ___size;
    int         ___status;
    std::string ___mangledName = ___symbol.substr(___firstBracket + 1, ___endOfName - ___firstBracket - 1);

    std::unique_ptr<char, void (*)(void*)> ___demangleResult{
      abi::__cxa_demangle(___mangledName.c_str(), nullptr, &___size, &___status),
      std::free
    };

    if (___status != 0) {
      continue;
    }

    ___retVal.location = ___symbol.substr(0, ___firstBracket);
    ___retVal.name     = ___demangleResult.get();
    // ___retVal.strAddress = ___symbol.substr(___addressStart + 1, ___addressEnd - ___addressStart - 1);
    ___retVal.address = (uintptr_t)_param_list[i];

    __prettyCallStacks.push_back(___retVal);
  }
#endif

  return __prettyCallStacks;
}

static inline uintptr_t GetBaseAddress() {
  uintptr_t pData;
#if !defined(PLATFORM_WINDOWS)
  auto _fnGetBaseAddrress = [](struct dl_phdr_info* info, size_t size, void* data) -> int {
    // Cast the data pointer back to the correct type
    uintptr_t* base_addr = static_cast<uintptr_t*>(data);
    // The main executable typically has an empty name in this list
    if (std::string(info->dlpi_name).empty()) {
      *base_addr = info->dlpi_addr;
      return 1; // Return non-zero to stop the iteration
    }
    return 0;
  };

  dl_iterate_phdr(_fnGetBaseAddrress, &pData);
#endif
  return pData;
};

void PrintLineNumber(void* addr) {
#if !defined(PLATFORM_WINDOWS)
  Dl_info info;

  // dladdr finds which file the address belongs to and its base address
  if (dladdr(addr, &info) && info.dli_fname) {
    // Calculate the relative offset
    uintptr_t relative_addr = (uintptr_t)addr - (uintptr_t)info.dli_fbase - 1;

    char command[512];
    // Use dli_fname to point to the correct file (it might be your .so!)
    snprintf(command, sizeof(command),
             "addr2line -e %s -Cip %p", // Added -C (demangle), -i (inlines), -p (pretty)
             info.dli_fname, (void*)relative_addr);

    FILE* fp = popen(command, "r");
    if (fp) {
      char buffer[256];
      if (fgets(buffer, sizeof(buffer), fp)) {
        std::printf("  At: %s", buffer);
      }
      pclose(fp);
    }
  }
#endif
}

void Debug::PrintCallStack() {
#if !defined(PLATFORM_WINDOWS)
  void*  array[50];
  int    size     = backtrace(array, 50);
  char** messages = backtrace_symbols(array, size);

  const uintptr_t baseAddress = GetBaseAddress();

  if (messages) {
    std::vector<SCallStack> listCallStacks = GetTranslatedCallStacks(array, messages, size);
    for (auto&& i : listCallStacks) {
      PrintLineNumber((void*)i.address);
    }
    free(messages);
  }
#endif
}