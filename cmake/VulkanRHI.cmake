# VulkanRHI module
#
# Module dependecies :
#   - rhi
#   - core
#   - Vulkan

add_library(vulkanrhi)

target_include_directories(vulkanrhi
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanRHI/Public/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanRHI/Public/OptimEngine/"
  PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanRHI/Private/"
)

target_sources(vulkanrhi PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanRHI/Private/VulkanRHI.cpp"
)

target_link_libraries(vulkanrhi PRIVATE
  Vulkan::Vulkan
  rhi
  core
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(vulkanrhi PRIVATE -fvisibility=hidden)
endif()