# knm::vulkan_tools

[![Documentation Status](https://readthedocs.org/projects/knm-vulkan-tools/badge/?version=latest)](https://knm-vulkan-tools.readthedocs.io/en/latest/?badge=latest)

Helper tools to implement a Vulkan-based C++ application. It handles the following
Vulkan operations:

- creation of a GLFW window
- creation of the Vulkan instance, enabling all the required extensions and the
  validation layers (for debug builds only)
- creation of a window surface, to be used by Vulkan to render into the GLFW
  window
- selection of the physical device to use (a graphics card)
- creation of the logical device and its queues
- creation of the swap chain, with several images

Additionaly, the destruction of all those objects at shutdown is implemented.

The implementation is heavily based on
[https://vulkan-tutorial.com/](https://vulkan-tutorial.com/). The idea is to
have a quick solution to jump-start a new project. A handful of examples are
available showing how to do common things in Vulkan.

Note that this library isn't a rendering engine/library or an abstraction layer on
top of Vulkan: the goal is to fasten a bit the repetitive task of configuring Vulkan
to render into a window.

The user is still expected to write the actual Vulkan code doing the rendering, which
means to implement its own management code for render passes, graphics pipelines,
framebuffers, vertex & index buffers, command buffers, etc...

Some settings can be modified by changing a configuration structure, other requires
to override some methods to implement your own version (if the choices made by this
implementation doesn't fulfill your needs). A lot of methods can be overriden (in
order to keep a maximum of flexibility), but most of them should be fine as they are
in most scenarios.

**knm::vulkan_tools** is a single-file, header-only C++ library. It relies on *GLFW*
([https://www.glfw.org/](https://www.glfw.org/)) to create and manage the window.

Everything is defined in the `knm::vk` namespace (`knm` for my username, *Kanma*,
and `vk` for *Vulkan*).


## Usage

1. Copy this file in a convenient place for your project

2. In *one* C++ file, add the following code to create the implementation:

```cpp
    #define KNM_VULKAN_TOOLS_IMPLEMENTATION
    #include <knm_vulkan_tools.hpp>
```

In other files, just use #include <knm_vulkan_tools.hpp>


The most simple usage is demonstrated in the "minimal" example, reproduced here:

```cpp
    #define KNM_VULKAN_TOOLS_IMPLEMENTATION
    #include <knm_vulkan_tools.hpp>

    #include <iostream>

    using namespace knm::vk;


    class MinimalApplication: public knm::vk::Application
    {
    protected:
        virtual void createVulkanObjects() override
        {
        }

        virtual void onSwapChainReady() override
        {
        }

        virtual uint32_t getNbCommandBuffers() const override
        {
            return 0;
        }

        virtual void getCommandBuffers(
            float elapsed, uint32_t imageIndex,
            std::vector<VkCommandBuffer>& outCommandBuffers
        ) override
        {
        }

        virtual void onSwapChainAboutToBeDestroyed() override
        {
        }

        virtual void destroyVulkanObjects() override
        {
        }
    };


    int main(int argc, char** argv)
    {
        MinimalApplication app;

        try
        {
            app.run();
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
```


This example just create and show a window, and configure Vulkan to render into it.
No rendering is done, since the user is expected to write this part.


## Dependencies

**knm::vulkan_tools** requires the following libraries:

- The Vulkan SDK ([https://vulkan.lunarg.com/](https://vulkan.lunarg.com/))
- GLFW ([https://www.glfw.org/](https://www.glfw.org/))

See [https://vulkan-tutorial.com/Development_environment](https://vulkan-tutorial.com/Development_environment)
if you need guidance about the setup of those dependencies and your project. The
**knm::vulkan_tools** repository also contains a *CMake* project configured to work on
Windows/Linux/MacOS.


In order to compile the examples (which is totally optional, since **knm::vulkan_tools**
is a single-file, header-only C++ library that doesn't require to be compiled separately
from your application), you'll also need:

- CMake ([https://cmake.org](https://cmake.org))
- GLM (*OpenGL Mathematics*, [https://github.com/g-truc/glm](https://github.com/g-truc/glm))


## License

knm::vulkan_tools is made available under the MIT License.

Copyright (c) 2023 Philip Abbet

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
