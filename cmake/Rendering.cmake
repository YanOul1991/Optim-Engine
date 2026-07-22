# VulkanRHI module
#
# Module dependecies :
#   - core
#   - rhi
#   - vulkanrhi
#

add_library(rendering)

target_include_directories(rendering 
  PRIVATE 
    "${CMAKE_SOURCE_DIR}/Source"
    "${CMAKE_SOURCE_DIR}/Source/Rendering/Private"
)

target_sources(rendering 
  PRIVATE
    "${CMAKE_SOURCE_DIR}/Source/Rendering/Private/Rendering.cpp"
)

# target_compile_definitions(rendering PRIVATE RENDERING_MODULE)

generate_export_header(rendering
  BASE_NAME rendering
  EXPORT_MACRO_NAME RENDERING_API
  EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Minimal/Generated/RenderModuleGenerated.h"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(rendering PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(rendering 
  PRIVATE 
    core
    rhi
    vulkanrhi
)