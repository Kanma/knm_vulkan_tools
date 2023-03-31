Usage
=====

1. Copy this file in a convenient place for your project

2. In *one* C++ file, add the following code to create the implementation:

.. code:: cpp

    #define KNM_VULKAN_TOOLS_IMPLEMENTATION
    #include <knm_vulkan_tools.hpp>

In other files, just use ``#include <knm_vulkan_tools.hpp>``


The most simple usage is demonstrated in the "minimal" example, reproduced here:

.. code:: cpp

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


This example just create and show a window, and configure Vulkan to render into it.
No rendering is done, since the user is expected to write this part.
