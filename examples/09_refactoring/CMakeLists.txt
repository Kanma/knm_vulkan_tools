include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/dependencies")

set(SRC_FILES
    main.cpp
    geometry.cpp
    image.cpp
    texture.cpp
    uniforms_buffer.cpp
)

set(HEADER_FILES
    geometry.h
    image.h
    texture.h
    uniforms_buffer.h
)

add_executable(refactoring ${SRC_FILES} ${HEADER_FILES})
target_link_libraries(refactoring Vulkan::Vulkan glfw)

compile_shaders(refactoring ${CMAKE_CURRENT_SOURCE_DIR}/shaders shader.vert shader.frag)
copy_models(refactoring viking_room.obj)
copy_textures(refactoring viking_room.png)
