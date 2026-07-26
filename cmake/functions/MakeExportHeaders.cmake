# Utiliy function to make generating export macros
# for modules easier.
# Simply need to pass the module name as a parameter
# and it will generate consistant code for each one.

include(GenerateExportHeader)

function(make_export_headers module_name)
  string(TOUPPER "${module_name}" MODNAME_UPPER)
  
  generate_export_header(${module_name}
    BASE_NAME ${module_name}
    EXPORT_MACRO_NAME "${MODNAME_UPPER}_API"
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/Public/OptimEngine/Core/Minimal/Generated/${module_name}_exports_generated.h"
  )
endfunction()