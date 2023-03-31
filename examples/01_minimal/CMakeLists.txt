include_directories("${PROJECT_SOURCE_DIR}")

add_executable(minimal main.cpp)
target_link_libraries(minimal Vulkan::Vulkan glfw)
set_target_properties(minimal PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/minimal)
