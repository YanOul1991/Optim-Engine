# Core module
#
# Module dependecies : This module has no dependencies

add_library(core)

target_include_directories(core
  PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/OptimEngine/"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Private/"
)

target_sources(core PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/System/LowLevel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/StandardTypes/String.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/StandardTypes/THashTable.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/Debug/Debug.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/Debug/Logging.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/Time/Time.cpp"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(core PRIVATE -fvisibility=hidden)
endif()
