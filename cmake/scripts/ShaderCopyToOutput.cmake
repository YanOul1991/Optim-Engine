# This script copies all source shaders modules to the output output
# directory under a `Shaders` subdir.
#
# Compilation is being done at runtime through the Slang compilation API.

if(NOT DEFINED SRC_DIR OR NOT DEFINED OUT_DIR)
  message(FATAL_ERROR "Missing arguments. Usage: cmake -DSRC_DIR=<path> -DOUT_DIR=<path> -P ShaderCopyToOutput.cmake")
endif()

# Make sure the output directory exists.
file(MAKE_DIRECTORY "${OUT_DIR}")

# Get all source .slang shader files.
file(GLOB_RECURSE SHADER_FILES "${SRC_DIR}/*.slang")

foreach(SHADER_PATH ${SHADER_FILES})
  file(RELATIVE_PATH REL_PATH "${SRC_DIR}" "${SHADER_PATH}")
  get_filename_component(REL_DIR "${REL_PATH}" DIRECTORY)
  get_filename_component(BASE_NAME ${SHADER_PATH} NAME_WE)
  get_filename_component(FILE_NAME ${SHADER_PATH} NAME)
  message(STATUS "[Slang] Copying ${BASE_NAME}.spv to output dirctory...")
  file(COPY ${SHADER_PATH} DESTINATION ${OUT_DIR})
endforeach()
