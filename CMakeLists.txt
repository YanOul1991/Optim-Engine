# CMake settings for Optim Engine

cmake_minimum_required(VERSION 4.2)

if(LINUX)
  set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/custom-triplets")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(BUILD_SHARED_LIBS ON)
  # set(VCPKG_CRT_LINKAGE "dynamic")

  if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows")
  elseif(LINUX)
    set(VCPKG_TARGET_TRIPLET "x64-linux-debug")
  endif()
else()
  set(BUILD_SHARED_LIBS OFF)
  # set(VCPKG_CRT_LINKAGE "static")

  if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static")
  endif()
endif()

project(OptimEngine VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output/${CMAKE_BUILD_TYPE}/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output/${CMAKE_BUILD_TYPE}/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output/${CMAKE_BUILD_TYPE}/lib")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_CURRENT_SOURCE_DIR}/output/${CMAKE_BUILD_TYPE}/")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_CURRENT_SOURCE_DIR}/output/${CMAKE_BUILD_TYPE}/")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_CURRENT_SOURCE_DIR}/output/${CMAKE_BUILD_TYPE}/lib")
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

# Compile slang shaders after build

function(add_slang_shader_target TARGET)
  set(SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
  set(SLANG_SOURCE "${SHADERS_DIR}/${TARGET}.slang")
  set(SPIRV_OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shaders/${TARGET}.spv")

  find_program(SLANGC
    NAMES slangc slangc.exe
    HINTS "$ENV{VULKAN_SDK}/bin" "$ENV{VULKAN_SDK}/Bin"
  )

  if(NOT SLANGC)
    message(FATAL_ERROR "Could not find slang compiler.")
  endif()

  add_custom_command(
    OUTPUT "${SPIRV_OUTPUT}"
    COMMAND "${SLANGC}"
            "${SLANG_SOURCE}"
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -entry VertexMain
            -entry FragmentMain
            -o "${SPIRV_OUTPUT}"
    DEPENDS "${SLANG_SOURCE}"
    COMMENT "Compiling Slang shader: ${TARGET}.slang"
    VERBATIM
  )

  add_custom_target(${TARGET} DEPENDS "${SPIRV_OUTPUT}")
  add_dependencies(optimengine ${TARGET})
endfunction()

add_slang_shader_target(sample)
