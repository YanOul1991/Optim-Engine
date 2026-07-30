# RHI module
#
# Module dependencies :
#   * core
#

add_library(rhi)

target_include_directories(rhi 
  PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/OptimEngine/"
  PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Private/"
)

target_sources(rhi PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/RHI.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/IGraphicsRHI.cpp"
)

target_link_libraries(rhi PRIVATE 
  core
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(rhi PRIVATE -fvisibility=hidden)
endif()

export_headers(rhi)