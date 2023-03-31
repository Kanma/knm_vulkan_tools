include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/dependencies")

add_executable(multisampling main.cpp)
target_link_libraries(multisampling Vulkan::Vulkan glfw)

compile_shaders(multisampling ${CMAKE_CURRENT_SOURCE_DIR}/shaders shader.vert shader.frag)
copy_models(multisampling viking_room.obj)
copy_textures(multisampling viking_room.png)
