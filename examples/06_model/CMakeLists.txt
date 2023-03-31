include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/dependencies")

add_executable(model main.cpp)
target_link_libraries(model Vulkan::Vulkan glfw)

compile_shaders(model ${CMAKE_CURRENT_SOURCE_DIR}/shaders shader.vert shader.frag)
copy_models(model viking_room.obj)
copy_textures(model viking_room.png)
