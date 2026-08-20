# CMake settings for Optim Engine

cmake_minimum_required(VERSION 4.2)

if(LINUX)
  set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/custom-triplets")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(BUILD_SHARED_LIBS ON)
  set(VCPKG_CRT_LINKAGE "dynamic")
  if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows")
  elseif(LINUX)
    set(VCPKG_TARGET_TRIPLET "x64-linux-debug")
  endif()
else()
  set(BUILD_SHARED_LIBS OFF)
  set(VCPKG_CRT_LINKAGE "static")
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

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# External System Libraries
find_package(Vulkan REQUIRED)

# Vcpkg Managed Libraries
find_package(SDL3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(slang CONFIG REQUIRED)

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


# Shaders Compilation
add_custom_target(shaders ALL)
add_dependencies(shaders optimengine)
add_custom_command(
  TARGET shaders POST_BUILD
  COMMAND ${CMAKE_COMMAND}
          -DSRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
          -DOUT_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}Shaders"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/ShaderCopyToOutput.cmake"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Compiling shaders..."
)

# Third Party Libraries 
add_custom_target(thirdpartylibs ALL)
add_dependencies(thirdpartylibs optimengine)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(WIN32)
    add_custom_command(
      TARGET thirdpartylibs POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_RUNTIME_DLLS:optimengine>" "$<TARGET_FILE_DIR:optimengine>"
      COMMAND_EXPAND_LISTS
    )
  else()
    add_custom_command(
      TARGET thirdpartylibs POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3::SDL3> "$<TARGET_FILE_DIR:optimengine>"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fmt::fmt> "$<TARGET_FILE_DIR:optimengine>"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:slang::slang> "$<TARGET_FILE_DIR:optimengine>"
    )
  endif()
endif()