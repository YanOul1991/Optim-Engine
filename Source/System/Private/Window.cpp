/**
 * Window.cpp
 */

#define SDL_MAIN_HANDLED

#include "OptimEngine/System/Window.h"

#include "SDL3/SDL.h"

#if PLATFORM_WINDOWS
#  pragma comment(lib, "setupapi.lib")
#  pragma comment(lib, "winmm.lib")
#  pragma comment(lib, "imm32.lib")
#  pragma comment(lib, "version.lib")
#endif

Window::Window(const char* windowName) : windowId(~0), pSDLWindow(nullptr)
{
  SDL_Window* pWin = SDL_CreateWindow(windowName, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_VULKAN);

  if (pWin != nullptr) {
    windowId   = SDL_GetWindowID(pWin);
    pSDLWindow = pWin;
  }
}

Window::~Window()
{
  SDL_DestroyWindow(static_cast<SDL_Window*>(pSDLWindow));
}
