# VulkanRHI module
#
# Module dependecies :
#   * Vulkan::Vulkan
#   * SDL3::SDL3
#   * rhi
#   * core

add_library(vulkanrhi)

target_include_directories(vulkanrhi
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/OptimEngine/"
  PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Private/"
)

target_sources(vulkanrhi PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/VulkanRHI.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/OptimVKSetup.cpp"
)

target_link_libraries(vulkanrhi PRIVATE
  Vulkan::Vulkan
  SDL3::SDL3
  rhi
  core
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(vulkanrhi PRIVATE -fvisibility=hidden)
endif()

export_headers(vulkanrhi)