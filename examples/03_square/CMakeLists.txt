include_directories("${PROJECT_SOURCE_DIR}")

add_executable(square main.cpp)
target_link_libraries(square Vulkan::Vulkan glfw)

compile_shaders(square ${CMAKE_CURRENT_SOURCE_DIR}/shaders shader.vert shader.frag)
