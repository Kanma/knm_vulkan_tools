Dependencies
============

**knm::vulkan_tools** requires the following libraries:

- The Vulkan SDK (https://vulkan.lunarg.com/)
- GLFW (https://www.glfw.org/)

See https://vulkan-tutorial.com/Development_environment if you need guidance about the
setup of those dependencies and your project. The **knm::vulkan_tools** repository
also contains a *CMake* project configured to work on Windows/Linux/MacOS.


In order to compile the examples (which is totally optional, since **knm::vulkan_tools**
is a single-file, header-only C++ library that doesn't require to be compiled separately
from your application), you'll also need:

- CMake (https://cmake.org)
- GLM (*OpenGL Mathematics*, https://github.com/g-truc/glm)
