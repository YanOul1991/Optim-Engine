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

target_link_libraries(optimengine PRIVATE
  system
  rhi
  vulkanrhi
  rendering
  core
)
