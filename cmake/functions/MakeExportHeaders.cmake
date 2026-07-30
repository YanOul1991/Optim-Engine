# THIS FILE SHOULD ONLY BE INCLUDED IN ROOT SOURCE DIR
#
# Utiliy function to make generating export macros for modules easier.
# Simply need to pass the module name as a parameter and it will generate 
# consistant code for each one.

# include(GenerateExportHeader)

# function(make_export_headers module_name)
#   string(TOUPPER "${module_name}" MODNAME_UPPER)
  
#   generate_export_header(${module_name}
#     BASE_NAME ${module_name}
#     EXPORT_MACRO_NAME "${MODNAME_UPPER}_API"
#     EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Public/OptimEngine/Core/Minimal/Generated/${module_name}_exports_generated.h"
#   )
# endfunction()

# add_library(engine_generated_headers INTERFACE)

# function(export_headers target_name)
#   string(TOUPPER "${target_name}" MODNAME_UPPER)

#   set(GEN_DIR "${CMAKE_BINARY_DIR}/OptimEngine/Core/Minimal/Generated")
#   set(GEN_FILE "${GEN_DIR}/${target_name}_exports_generated.h")
  
#   generate_export_header(${target_name}
#     BASE_NAME ${target_name}
#     EXPORT_MACRO_NAME "${MODNAME_UPPER}_API"
#     EXPORT_FILE_NAME "${GEN_FILE}"
#   )
  
#   # target_include_directories(engine_generated_headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
# endfunction()

# set(ENGINE_EXPORT_MACROS "" CACHE INTERNAL "Holds generated export macros")

function(export_headers target_name)
    string(TOUPPER "${target_name}" MOD_UPPER)

    target_compile_definitions(${target_name} PRIVATE "EXPORT_${MOD_UPPER}")


    # Generate the exact standard C++ visibility blocks manually
    set(MACRO_BLOCK "
// Export macros for ${target_name} module
#if !defined(${MOD_UPPER}_API)
#  if defined(EXPORT_${MOD_UPPER})
#    if defined(_MSC_VER) || defined(__CYGWIN__)
#      define ${MOD_UPPER}_API __declspec(dllexport)
#    else
#      define ${MOD_UPPER}_API __attribute__((visibility(\"default\")))
#    endif
#  else
#    if defined(_MSC_VER) || defined(__CYGWIN__)
#      define ${MOD_UPPER}_API __declspec(dllimport)
#    else
#      define ${MOD_UPPER}_API
#    endif
#  endif
#endif")

    # set(CURRENT_MACROS "${ENGINE_EXPORT_MACROS}\\${MACRO_BLOCK}")
    # set(ENGINE_EXPORT_MACROS "${CURRENT_MACROS}" CACHE INTERNAL "Holds generated export macros")

    set_property(GLOBAL APPEND PROPERTY EXPORT_ENGINE_MACROS "${MACRO_BLOCK}")

endfunction()

# Defer generation of header file to make sure that every module registered their macros.

function(generate_master_header)
    get_property(COLLECTED_MACROS GLOBAL PROPERTY EXPORT_ENGINE_MACROS)

    # Add any extra global data you want in the future here (e.g., build timestamps)
    set(HEADER_CONTENT "// THIS FILE IS AUTO-GENERATED.\n")
    string(APPEND HEADER_CONTENT "#pragma once\n")
    string(APPEND HEADER_CONTENT "${COLLECTED_MACROS}")

    # Write the file directly into the Core module's binary directory layout
    set(OUTPUT_FILE "${CMAKE_SOURCE_DIR}/Source/Core/Public/OptimEngine/Core/Minimal/ModuleExportMacros.h")
    
    file(GENERATE OUTPUT "${OUTPUT_FILE}" CONTENT "${HEADER_CONTENT}")
endfunction()

cmake_language(DEFER CALL generate_master_header)
