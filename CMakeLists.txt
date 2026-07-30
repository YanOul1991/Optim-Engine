# CMake settings for Optim Engine

cmake_minimum_required(VERSION 4.2)

project(OptimEngine VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

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
# LINKAGE OPTIONS
###############################################################################

# # Turn off CMake's dynamic internal RPATH rewrites entirely
# set(CMAKE_SKIP_RPATH TRUE)
# # Tell the system compiler to bake exactly '$ORIGIN' into ALL executables
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'\$ORIGIN' -Wl,--disable-new-dtags")
# # Tell the system compiler to bake exactly '$ORIGIN' into ALL shared libraries
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,'\$ORIGIN' -Wl,--disable-new-dtags")

# 1. Enable RPATH for your build tree 
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# 2. Do not clear the RPATH when installing/packaging the binaries
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# 3. Use $ORIGIN so it searches the executable's own directory first
# (The escaped '\$ORIGIN' is critical so CMake passes it as a literal string to the linker)
set(CMAKE_INSTALL_RPATH "\$ORIGIN")

# 4. Automatically add dependent library paths to the runtime search path
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


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

add_subdirectory("Source/VulkanRHI")
add_subdirectory("Source/Core")
add_subdirectory("Source/RHI")
add_subdirectory("Source/Rendering")
add_subdirectory("Source/System")
add_subdirectory("Source/Application")

###############################################################################
# EXPORT MACROS GENERATION
###############################################################################
# make_export_headers(core)
# make_export_headers(rendering)
# make_export_headers(rhi)
# make_export_headers(system)
# make_export_headers(vulkanrhi)
