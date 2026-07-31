/**
 * Window.cpp
 */

#define SDL_MAIN_HANDLED

#include "System/Window.h"

#include "SDL3/SDL.h"

#if defined(_WIN32) || defined(_WIN64)
#  include <Windows.h>
#  pragma comment(lib, "setupapi.lib")
#  pragma comment(lib, "winmm.lib")
#  pragma comment(lib, "imm32.lib")
#  pragma comment(lib, "version.lib")
#endif

// Main window
SDL_Window* s_mainWindow = nullptr;

Window::Window(const char* windowName)
{
  s_mainWindow = SDL_CreateWindow(windowName, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_VULKAN);

  if (s_mainWindow) {
    m_WindowID = SDL_GetWindowID(s_mainWindow);
  }
}

Window::~Window()
{
  SDL_DestroyWindow(s_mainWindow);
}

bool Window::IsValid() const
{
  return s_mainWindow != nullptr;
}
