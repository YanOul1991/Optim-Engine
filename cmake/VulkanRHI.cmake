# VulkanRHI module
#
# Module dependecies :
#   - rhi
#   - core
#

cmake_minimum_required(VERSION 4.2)

find_package(Vulkan REQUIRED)

add_library(vulkanrhi)

target_include_directories(vulkanrhi
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanRHI/Private/"
)

target_sources(vulkanrhi 
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanRHI/Private/VulkanRHI.cpp"
)

# target_compile_definitions(vulkanrhi PRIVATE VULKANRHI_MODULE)

generate_export_header(vulkanrhi
  BASE_NAME vulkanrhi
  EXPORT_MACRO_NAME VULKANRHI_API
  EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Minimal/Generated/VulkanRhiModuleGenerated.h"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(vulkanrhi PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(vulkanrhi
  PRIVATE
    Vulkan::Vulkan
    rhi
    core
)