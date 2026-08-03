/**
 * main.cpp
 *
 * This file contains the prorgam entry and the global logic
 * for the program such as initialization logic, looping and
 * quit.
 */

// Core module
#include "OptimEngine/Core/Memory/TUniquePtr.h"
#include "OptimEngine/Core/StandardTypes/String.h"
#include "OptimEngine/Core/Time/Time.h"

// Graphics related modules
#include "OptimEngine/RHI/RHI.h"
#include "OptimEngine/Rendering/Rendering.h"
#include "OptimEngine/VulkanRHI/VulkanRHI.h"

// System module
#include "OptimEngine/System/System.h"
#include "OptimEngine/System/Window.h"

// FMT test
#include <fmt/color.h>

// STD HEADERS
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
#if PLATFORM_WINDOWS
#  include <windows.h>
// windows.h goes first ^^^^^
#  include <dbghelp.h>
#  pragma comment(lib, "dbghelp.lib")
#endif

void PrintCallStack()
{
  void* stack[62];
  // 1. Capture stack frames (skip 0 frames, capture up to 62)
  USHORT frames = CaptureStackBackTrace(0, 62, stack, NULL);

  HANDLE process = GetCurrentProcess();

  // 2. Initialize symbol handler and load the .pdb file automatically
  SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
  SymInitialize(process, NULL, TRUE);

  // Allocate memory for symbol info
  SYMBOL_INFO* symbol  = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
  symbol->MaxNameLen   = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  IMAGEHLP_LINE64 line;
  line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
  DWORD displacement;

  std::cout << "\n========== CALL STACK ==========\n";
  for (USHORT i = 0; i < frames; i++) {
    DWORD64 address = (DWORD64)(stack[i]);

    // Translate address -> Function Name
    BOOL hasSymbol = SymFromAddr(process, address, 0, symbol);

    // Translate address -> File Name & Line Number
    BOOL hasLine = SymGetLineFromAddr64(process, address, &displacement, &line);

    if (hasSymbol && hasLine) {
      std::cout << "[" << i << "] " << symbol->Name
                << "() -> " << line.FileName
                << ":" << line.LineNumber << "\n";
    }
    else if (hasSymbol) {
      std::cout << "[" << i << "] " << symbol->Name << "()\n";
    }
    else {
      std::cout << "[" << i << "] 0x" << std::hex << address << std::dec << "\n";
    }
  }
  std::cout << "================================\n\n";

  free(symbol);
  SymCleanup(process);
}
LONG WINAPI CrashHandler(EXCEPTION_POINTERS* exceptionInfo)
{
  HANDLE file = CreateFileA("crashdump.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file != INVALID_HANDLE_VALUE) {
    MINIDUMP_EXCEPTION_INFORMATION info;
    info.ThreadId          = GetCurrentThreadId();
    info.ExceptionPointers = exceptionInfo;
    info.ClientPointers    = TRUE;

    // Write the crash dump
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, &info, NULL, NULL);
    CloseHandle(file);
  }
  PrintCallStack();
  return EXCEPTION_EXECUTE_HANDLER;
}
*/

class Bar
{
 public:
  void Foo()
  {
    fmt::println(fg(fmt::color::green) | fmt::emphasis::bold, "System Module has been initialized");
  }
};

int main(int argc, char* argv[])
{
  // SetUnhandledExceptionFilter(CrashHandler);

  Bar bar;
  System::OnSystemModuleInitialized.SubscribeMemberFunction<&Bar::Foo>(&bar);

  // // --- Simulating a crash ---
  // int* badPointer = nullptr;
  // *badPointer     = 42; // Access Violation!

  // Initalize the System module
  System::Initalize();

  // Load the main window with the name of the application
  Window mainWindow = Window("Vulkan Engine Project");

  Internal::RenderModule rhiBackend(new VulkanRHI());

  rhiBackend.rhi.GetPtr()->Initialize(mainWindow.GetSDLWindowHandle());

  uint64 loopCycles = 0;

  while (System::ProcessEvents()) {
    loopCycles++;
  }

  std::cout << "Cycles count: " << loopCycles << '\n';

  mainWindow.~Window();

  System::Quit();

  return 0;
}