# # RHI module
# #
# # Module dependecies :
# #   - core module
# #

# add_library(rhi)

# target_include_directories(rhi 
#   PUBLIC 
#     "${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI/Public/"
#     "${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI/Public/OptimEngine/"
#   PRIVATE 
#     "${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI/Private/"
# )

# target_sources(rhi PRIVATE
#   "${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI/Private/RHI.cpp"
#   "${CMAKE_CURRENT_SOURCE_DIR}/Source/RHI/Private/IGraphicsRHI.cpp"
# )

# if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
#   target_compile_options(rhi PRIVATE -fvisibility=hidden)
# endif()

# target_link_libraries(rhi PRIVATE 
#   core
# )