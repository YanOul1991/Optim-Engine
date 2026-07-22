# cmake -B build | 
# cmake --build build | 
# cmake --build build/ --target clean
# cmake --build build --config Debug
# cmake -B build -DCMAKE_BUILD_TYPE=Debug

cmake_minimum_required(VERSION 4.2)

project(OptimEngine VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

# Static and shared libraries have no prefixes being set
# so that the output name is set as the given cmake
# library name
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(BUILD_SHARED_LIBS ON)

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

# COMPILATION FLAGS      
#   - Generate Debug Info
#     (GCC/CLANG) -> g[0-3]
#     (MSVC)
#     
#   - Optimization level
#       -O[0-3]
# 
# LINKER FLAGS
#   - rdynamic to all link steps
#
#   - Link-Time Optimizations: 
#       (GCC/CLANG) -> -flto
#       (MSVC)      -> /GL
#
if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /MD")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
endif()

# add_compile_options(-g3 -03)


include(GenerateExportHeader)
# include(FetchContent)

# THIRD PARTY LIBRARIES FETCH
# FetchContent_Declare(
#   SDL3
#   GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
#   GIT_TAG main         
# )

# set(SDL_TEST OFF CACHE BOOL "" FORCE)
# set(SDL_SHARED ON CACHE BOOL "" FORCE)

# FetchContent_MakeAvailable(SDL3)

# List all libraries in the `cmake` directory
# then include files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Core)
include(RHI)
include(VulkanRHI)
include(Rendering)
include(System)

find_package(SDL3 CONFIG REQUIRED)

# EXECUTABLE
#
# For the module(shared objects) depenencies of the executable, while
# it can depend on all of them, only link against its direct dependency
# for easier management and debugging in case of issues.

add_executable(optimengine)

target_include_directories(optimengine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source/Private")

target_sources(optimengine PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/Application/Private/main.cpp"
)

target_link_libraries(optimengine PRIVATE
  core
  rendering
  system
)

# Move all dlls into the samke directory as the executable if not already there.

add_custom_command(TARGET optimengine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_RUNTIME_DLLS:optimengine>"
    "$<TARGET_FILE_DIR:optimengine>"
  COMMAND_EXPAND_LISTS
)

# add_custom_command(TARGET optimengine POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy_if_different
#   "$<TARGET_PROPERTY:SDL3,IMPORTED_LOCATION>"
#   "$<TARGET_FILE_DIR:optimengine>"
#   COMMENT "Copying SDL3.dll to output directory..."
# )
