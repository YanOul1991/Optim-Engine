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
  # fmt::fmt
)

###############################################################################
# POST BUILD
# 
# On Windows, if in Debug, copy all required dlls into the same output
# folder as the executable, if they are not already there.
###############################################################################
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_custom_command(TARGET optimengine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_RUNTIME_DLLS:optimengine>"
      "$<TARGET_FILE_DIR:optimengine>"
    COMMAND_EXPAND_LISTS
  )
endif()

if(LINUX)
  add_custom_command(TARGET optimengine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3> 
        "${PROJECT_SOURCE_DIR}/output/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:fmt::fmt> 
        "${PROJECT_SOURCE_DIR}/output/"
)
endif()
