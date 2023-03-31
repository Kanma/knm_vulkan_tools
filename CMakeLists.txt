cmake_minimum_required(VERSION 3.22)
project(KNM_VULKAN_TOOLS)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

include("cmake/models.cmake")
include("cmake/shaders.cmake")
include("cmake/textures.cmake")

add_subdirectory(examples)
add_subdirectory(docs)
