# The following variables contains the files used by the different stages of the build process.
set(assignment1_default_default_XC16_FILE_TYPE_assemble)
set(assignment1_default_default_XC16_FILE_TYPE_assemblePreproc)
set_source_files_properties(${assignment1_default_default_XC16_FILE_TYPE_assemblePreproc} PROPERTIES LANGUAGE C)
set(assignment1_default_default_XC16_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../parser.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../timer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../uart.c")
set(assignment1_default_default_XC16_FILE_TYPE_link)

# The (internal) path to the resulting build image.
set(assignment1_default_internal_image_name "${CMAKE_CURRENT_SOURCE_DIR}/../../../_build/assignment1/default/default.elf")

# The name of the resulting image, including namespace for configuration.
set(assignment1_default_image_name "assignment1_default_default.elf")

# The name of the image, excluding the namespace for configuration.
set(assignment1_default_original_image_name "default.elf")

# The output directory of the final image.
set(assignment1_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/assignment1")
