function(copy_models target)
    set(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target})
    set(MODELS_DIR ${OUTPUT_DIR}/models)

    add_custom_target(make_${target}_models_directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${MODELS_DIR})

    set(outputs "")

    foreach(model ${ARGN})
        add_custom_command(
            OUTPUT ${MODELS_DIR}/${model}
            COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/models/${model} ${MODELS_DIR}/${model}
            DEPENDS ${PROJECT_SOURCE_DIR}/models/${model}
            VERBATIM
        )

        list(APPEND outputs ${MODELS_DIR}/${model})
    endforeach()

    add_custom_target(copy_${target}_models DEPENDS ${outputs})
    add_dependencies(copy_${target}_models make_${target}_models_directory)

    add_dependencies(${target} copy_${target}_models)
endfunction()
