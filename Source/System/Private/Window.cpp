/**
 * Window.cpp
 */

#define SDL_MAIN_HANDLED

#if defined(_WIN32) || defined(_WIN64)
#  pragma comment(lib, "setupapi.lib")
#  pragma comment(lib, "winmm.lib")
#  pragma comment(lib, "imm32.lib")
#  pragma comment(lib, "version.lib")
#endif

#include "System/Window.h"

// #include "Core/StandardTypes/TDynamicArray.h"
#include "SDL3/SDL.h"

// Main window
SDL_Window* s_mainWindow = nullptr;

// Default SDL window  flags
static constexpr SDL_WindowFlags s_defaultWindowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_VULKAN;

Window::Window(const char* windowName) {
  s_mainWindow = SDL_CreateWindow(windowName, 1280, 720, s_defaultWindowFlags);

  if (s_mainWindow) {
    m_WindowID = SDL_GetWindowID(s_mainWindow);
  }
}

uint32 Window::GetWindowID() const {
  return m_WindowID;
}

Window::operator bool() const {
  return s_mainWindow != nullptr;
}

bool Window::IsValid() const {
  return operator bool();
}
