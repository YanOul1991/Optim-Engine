# Rendering module
#
# Module dependencies :
#   * core
#   * rhi
#   * vulkanrhi
#

add_library(rendering)

target_include_directories(rendering 
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/OptimEngine/"
  PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Private/"
)

target_sources(rendering PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/Rendering.cpp"
)

target_link_libraries(rendering PRIVATE 
  core
  rhi
  vulkanrhi
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(rendering PRIVATE -fvisibility=hidden)
endif()

export_headers(rendering)