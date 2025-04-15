include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(assignment1_default_library_list )

# Handle files with suffix s, for group default-XC16
if(assignment1_default_default_XC16_FILE_TYPE_assemble)
add_library(assignment1_default_default_XC16_assemble OBJECT ${assignment1_default_default_XC16_FILE_TYPE_assemble})
    assignment1_default_default_XC16_assemble_rule(assignment1_default_default_XC16_assemble)
    list(APPEND assignment1_default_library_list "$<TARGET_OBJECTS:assignment1_default_default_XC16_assemble>")
endif()

# Handle files with suffix S, for group default-XC16
if(assignment1_default_default_XC16_FILE_TYPE_assemblePreproc)
add_library(assignment1_default_default_XC16_assemblePreproc OBJECT ${assignment1_default_default_XC16_FILE_TYPE_assemblePreproc})
    assignment1_default_default_XC16_assemblePreproc_rule(assignment1_default_default_XC16_assemblePreproc)
    list(APPEND assignment1_default_library_list "$<TARGET_OBJECTS:assignment1_default_default_XC16_assemblePreproc>")
endif()

# Handle files with suffix c, for group default-XC16
if(assignment1_default_default_XC16_FILE_TYPE_compile)
add_library(assignment1_default_default_XC16_compile OBJECT ${assignment1_default_default_XC16_FILE_TYPE_compile})
    assignment1_default_default_XC16_compile_rule(assignment1_default_default_XC16_compile)
    list(APPEND assignment1_default_library_list "$<TARGET_OBJECTS:assignment1_default_default_XC16_compile>")
endif()

add_executable(${assignment1_default_image_name} ${assignment1_default_library_list})

target_link_libraries(${assignment1_default_image_name} PRIVATE ${assignment1_default_default_XC16_FILE_TYPE_link})

# Add the link options from the rule file.
assignment1_default_link_rule(${assignment1_default_image_name})

# Add bin2hex target for converting built file to a .hex file.
add_custom_target(assignment1_default_Bin2Hex ALL
    ${MP_BIN2HEX} ${assignment1_default_image_name})
add_dependencies(assignment1_default_Bin2Hex ${assignment1_default_image_name})

# Post build target to copy built file to the output directory.
add_custom_command(TARGET ${assignment1_default_image_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${assignment1_default_output_dir}
                    COMMAND ${CMAKE_COMMAND} -E copy ${assignment1_default_image_name} ${assignment1_default_output_dir}/${assignment1_default_original_image_name}
                    BYPRODUCTS ${assignment1_default_output_dir}/${assignment1_default_original_image_name})
