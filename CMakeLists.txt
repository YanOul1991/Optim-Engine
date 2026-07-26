# CMake settings for Optim Engine

cmake_minimum_required(VERSION 4.2)

project(OptimEngine VERSION 1.0.0)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)


# All build file will be outputed to `output` directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/lib")

# Multi-config override (Visual Studio, Xcode, Ninja Multi-Config)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_SOURCE_DIR}/output")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_SOURCE_DIR}/output")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_SOURCE_DIR}/output/lib")
endforeach()


###############################################################################
# External System Libraries
###############################################################################
find_package(Vulkan REQUIRED)


###############################################################################
# Vcpkg Managed Libraries
###############################################################################
find_package(SDL3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)


# List all libraries in the `cmake` directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Functions
include(functions/MakeExportHeaders)

# Modules
include(Core)
include(RHI)
include(VulkanRHI)
include(Rendering)
include(System)


###############################################################################
# EXECUTABLE
# 
# For the module(shared objects) depenencies of the executable, while
# it can depend on all of them, only link against its direct dependency
# for easier management and debugging in case of issues.
###############################################################################
add_executable(optimengine)

target_include_directories(optimengine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source/Private")

target_sources(optimengine PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/Application/Private/main.cpp"
)

target_link_libraries(optimengine PRIVATE
  core
  rhi
  rendering
  system
)


###############################################################################
# EXPORT MACROS GENERATION
###############################################################################
make_export_headers(core)
make_export_headers(rendering)
make_export_headers(rhi)
make_export_headers(system)
make_export_headers(vulkanrhi)


###############################################################################
# POST BUILD
# 
# On Windows, if in Debug, copy all required dlls into the same output
# folder as the executable, if they are not already there.
###############################################################################
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_custom_command(TARGET optimengine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_RUNTIME_DLLS:optimengine>"
      "$<TARGET_FILE_DIR:optimengine>"
    COMMAND_EXPAND_LISTS
  )
endif()
