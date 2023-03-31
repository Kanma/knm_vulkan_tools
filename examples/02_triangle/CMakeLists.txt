include_directories("${PROJECT_SOURCE_DIR}")

add_executable(triangle main.cpp)
target_link_libraries(triangle Vulkan::Vulkan glfw)

compile_shaders(triangle ${CMAKE_CURRENT_SOURCE_DIR}/shaders shader.vert shader.frag)
