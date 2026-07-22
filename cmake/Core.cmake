# Core module
#
# Module dependecies : This module has no dependencies

add_library(core)

target_include_directories(core
  PUBLIC 
    "${CMAKE_SOURCE_DIR}/Source/"
  PRIVATE
    "${CMAKE_SOURCE_DIR}/Source/Core/"
)

target_sources(core
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/System/LowLevel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/System/Time.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/StandardTypes/String.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/StandardTypes/THashTable.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Debug/Private/Debug.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Debug/Private/Logging.cpp"
)

generate_export_header(core
  BASE_NAME core
  EXPORT_MACRO_NAME CORE_API
  EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Minimal/Generated/CoreModuleGenerated.h"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(core PRIVATE -fvisibility=hidden)
endif()
