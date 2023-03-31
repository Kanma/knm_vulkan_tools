include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/dependencies")

add_executable(mipmaps main.cpp)
target_link_libraries(mipmaps Vulkan::Vulkan glfw)

compile_shaders(mipmaps ${CMAKE_CURRENT_SOURCE_DIR}/shaders shader.vert shader.frag)
copy_models(mipmaps viking_room.obj)
copy_textures(mipmaps viking_room.png)
