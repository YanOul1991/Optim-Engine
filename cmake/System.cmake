# System module
#
# Module dependecies :
#   - core
#   - SDL (Third party)
#

add_library(system)

target_include_directories(system 
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/"
  PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/System/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/System/Private/"
)

target_sources(system PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/System/Private/SharedObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/System/Private/System.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/System/Private/Window.cpp"
)

generate_export_header(system
  BASE_NAME system
  EXPORT_MACRO_NAME SYSTEM_API
  EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Minimal/Generated/SystemModuleGenerated.h"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(system PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(system PRIVATE 
  SDL3::SDL3
  core
)