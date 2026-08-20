# EXECUTABLE
# 
# For the module(shared objects) depenencies of the executable, while
# it can depend on all of them, only link against its direct dependency
# for easier management and debugging in case of issues.
# 
# Module dependencies:
#   * system
#   * rhi
#   * vulkanrhi
#   * rendering
#   * core
#   * fmt::fmt

add_executable(optimengine)

target_include_directories(optimengine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private/")

target_sources(optimengine PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/main.cpp"
)

# set(ENV_VK_SDK_PATH $ENV{VULKAN_SDK})
# cmake_path(CONVERT "${ENV_VK_SDK_PATH}" TO_CMAKE_PATH_LIST VK_SDK_PATH)

# message(STATUS "${VK_SDK_PATH}")

# find_package(glslang CONFIG REQUIRED
#   HINTS "${VK_SDK_PATH}/lib/cmake"
# )

target_link_libraries(optimengine PUBLIC
  system
  rhi
  vulkanrhi
  rendering
  core
  Vulkan::Vulkan
  slang::slang
)
