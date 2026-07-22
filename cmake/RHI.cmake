# RHI module
#
# Module dependecies :
#   - core module
#

add_library(rhi)

target_include_directories(rhi 
  PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/"
  PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI/Private/"
)

target_sources(rhi PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI/Private/RHI.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI/Private/IGraphicsRHI.cpp"
)

generate_export_header(rhi
  BASE_NAME rhi
  EXPORT_MACRO_NAME RHI_API
  EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Minimal/Generated/RhiModuleGenerated.h"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(rhi PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(rhi PRIVATE 
  core
)