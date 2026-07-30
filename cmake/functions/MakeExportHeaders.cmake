# THIS FILE SHOULD ONLY BE INCLUDED IN ROOT SOURCE DIR
#
# Utiliy function to make generating export macros for modules easier.
# Simply need to pass the module name as a parameter and it will generate 
# consistant code for each one.

function(export_headers target_name)
    string(TOUPPER "${target_name}" MOD_UPPER)

    # Add compile definition so the macro expands to the correct visibility 
    # attribute in the correct module.
    target_compile_definitions(${target_name} PRIVATE "EXPORT_${MOD_UPPER}")

    # C++ visibility block template
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
