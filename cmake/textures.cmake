function(copy_textures target)
    set(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target})
    set(TEXTURES_DIR ${OUTPUT_DIR}/textures)

    add_custom_target(make_${target}_textures_directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${TEXTURES_DIR})

    set(outputs "")

    foreach(texture ${ARGN})
        add_custom_command(
            OUTPUT ${TEXTURES_DIR}/${texture}
            COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/textures/${texture} ${TEXTURES_DIR}/${texture}
            DEPENDS ${PROJECT_SOURCE_DIR}/textures/${texture}
            VERBATIM
        )

        list(APPEND outputs ${TEXTURES_DIR}/${texture})
    endforeach()

    add_custom_target(copy_${target}_textures DEPENDS ${outputs})
    add_dependencies(copy_${target}_textures make_${target}_textures_directory)

    add_dependencies(${target} copy_${target}_textures)
endfunction()
