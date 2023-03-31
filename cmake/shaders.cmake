function(compile_shaders target folder)
    set(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target})
    set(SHADERS_DIR ${OUTPUT_DIR}/shaders)

    add_custom_target(make_${target}_shaders_directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR})

    set(outputs "")

    foreach(shader ${ARGN})
        add_custom_command(
            OUTPUT ${SHADERS_DIR}/${shader}.spv
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${shader} -o ${SHADERS_DIR}/${shader}.spv
            DEPENDS ${folder}/${shader}
            WORKING_DIRECTORY ${folder}
            VERBATIM
        )

        list(APPEND outputs ${SHADERS_DIR}/${shader}.spv)
    endforeach()

    add_custom_target(compile_${target}_shaders DEPENDS ${outputs})
    add_dependencies(compile_${target}_shaders make_${target}_shaders_directory)

    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
    add_dependencies(${target} compile_${target}_shaders)
endfunction()
