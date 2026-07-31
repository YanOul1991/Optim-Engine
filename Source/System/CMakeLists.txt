# System module
#
# Module dependecies :
#   * SDL3::SDL3
#   * core

add_library(system)

target_include_directories(system 
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/"
    "${CMAKE_CURRENT_SOURCE_DIR}/Public/OptimEngine/"
  PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Private/"
)

target_sources(system PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/SharedObject.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/System.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/Window.cpp"
)

target_link_libraries(system PRIVATE 
  SDL3::SDL3
  core
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(system PRIVATE -fvisibility=hidden)
endif()

if(VCPKG_TARGET_TRIPLET MATCHES "static")
  target_compile_definitions(system PRIVATE SDL_STATIC)
endif()

export_headers(system)