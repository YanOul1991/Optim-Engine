# This cmake scripts compiles the .slang source shaders in the Shaders directory
# of the project into spv binaries.
#
# Shaders are compiled in the same directory as the source files, but then copied to the
# executable output directory under a `Shaders` directory to be used by the application.
#
# This script is only to be called in the root level CMake source file.

if(NOT DEFINED SRC_DIR OR NOT DEFINED OUT_DIR)
  message(FATAL_ERROR "Missing arguments. Usage: cmake -DSRC_DIR=<path> -DOUT_DIR=<path> -P CompileShaders.cmake")
endif()

# Local the Slang compiler (slangc). With the Vulkan SDK its directory should 
# be in the system's path variable, but just in case, use the `VULKAN_SDK` 
# system symbol that should always be defined when using Vulkan.
find_program(SLANGC_EXEC
  NAMES slangc slangc.exe REQUIRED
  HINTS "$ENV{VULKAN_SDK}/bin" "$ENV{VULKAN_SDK}/Bin"
)

# Make sure the output directory exists.
file(MAKE_DIRECTORY "${OUT_DIR}")

# Get all source .slang shader files.
file(GLOB_RECURSE SHADER_FILES "${SRC_DIR}/*.slang")

foreach(SHADER_PATH ${SHADER_FILES})
  # Get the relative path for the file, relative to the `Shaders` directory.
  file(RELATIVE_PATH REL_PATH "${SRC_DIR}" "${SHADER_PATH}")

  get_filename_component(REL_DIR "${REL_PATH}" DIRECTORY)
  # Name without extension
  get_filename_component(BASE_NAME ${SHADER_PATH} NAME_WE)
  # Name WITH extension
  get_filename_component(FILE_NAME ${SHADER_PATH} NAME)

  if(${REL_PATH})
    set(COMPILED_LOCAL "${SRC_DIR}/${REL_DIR}/${BASE_NAME}.spv")
    else()
    set(COMPILED_LOCAL "${SRC_DIR}/${BASE_NAME}.spv")
  endif()

  message(STATUS "[Slang] Compiling ${FILE_NAME}")
  
  execute_process(
    COMMAND "${SLANGC_EXEC}" 
            "${SHADER_PATH}"
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -entry VertexMain
            -entry FragmentMain
            -o "${COMPILED_LOCAL}"
    RESULT_VARIABLE COMPILE_RESULT
  )

  if(NOT COMPILE_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to compile shader: ${FILE_NAME}")
  endif()

  message(STATUS "[Slang] Copying ${BASE_NAME}.spv to output dirctory...")
  file(COPY ${COMPILED_LOCAL} DESTINATION ${OUT_DIR})
  file(COPY ${SHADER_PATH} DESTINATION ${OUT_DIR})

endforeach()
