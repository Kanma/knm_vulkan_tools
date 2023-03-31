include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/dependencies")

add_executable(depth main.cpp)
target_link_libraries(depth Vulkan::Vulkan glfw)

compile_shaders(depth ${CMAKE_CURRENT_SOURCE_DIR}/shaders shader.vert shader.frag)
copy_textures(depth m31.jpg)
