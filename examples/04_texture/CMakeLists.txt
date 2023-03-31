include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/dependencies")

add_executable(texture main.cpp)
target_link_libraries(texture Vulkan::Vulkan glfw)

compile_shaders(texture ${CMAKE_CURRENT_SOURCE_DIR}/shaders shader.vert shader.frag)
copy_textures(texture m31.jpg)
