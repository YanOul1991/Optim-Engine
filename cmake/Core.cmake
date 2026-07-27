# Core module
#
# Module dependecies : This module has no dependencies

add_library(core)

target_include_directories(core
  PUBLIC 
    "${CMAKE_SOURCE_DIR}/Source/Core/Public/"
    "${CMAKE_SOURCE_DIR}/Source/Core/Public/OptimEngine"
  PRIVATE
    "${CMAKE_SOURCE_DIR}/Source/Core/Private/"
)

target_sources(core PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Private/System/LowLevel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Private/StandardTypes/String.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Private/StandardTypes/THashTable.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Private/Debug/Debug.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Private/Debug/Logging.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Private/Time/Time.cpp"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(core PRIVATE -fvisibility=hidden)
endif()
