/*
SPDX-FileCopyrightText: 2023 Philip Abbet

SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>

SPDX-License-Identifier: MIT
*/

/** Minimal example

This example shows the minimal amount of code needed to use the library, but doesn't
renders anything.
*/


#define KNM_VULKAN_TOOLS_IMPLEMENTATION
#include <knm_vulkan_tools.hpp>

#include <iostream>

using namespace knm::vk;


//----------------------------------------------------------------------------------------
// The user must inherit from the knm::vk::Application class, and implement a few methods
// to create its own Vulkan objects (render passes, graphics pipelines, framebuffers,
// vertex & index buffers, command buffers, ...) and do the actual rendering.
//----------------------------------------------------------------------------------------
class MinimalApplication: public knm::vk::Application
{
protected:
    //------------------------------------------------------------------------------------
    // Method called after everything was initialised (window, instance, logical device,
    // swap chain), right before entering the main loop.
    //
    // Use it to create your own Vulkan objects (render passes, graphics pipelines,
    // vertex & index buffers, command buffers, ...).
    //------------------------------------------------------------------------------------
    virtual void createVulkanObjects() override
    {
    }

    //------------------------------------------------------------------------------------
    // Method called after the swap chain was created (will happen each time the window
    // is resized and at application startup).
    //
    // Use it to create your own Vulkan objects (like the framebuffers) that depends on
    // the swap chain (ie. the dimensions and number of its images).
    //------------------------------------------------------------------------------------
    virtual void onSwapChainReady() override
    {
    }

    //--------------------------------------------------------------------------------
    // Method called to retrieve the number of command buffers that need to be executed
    // to render the current frame.
    //--------------------------------------------------------------------------------
    virtual uint32_t getNbCommandBuffers() const override
    {
        return 0;
    }

    //------------------------------------------------------------------------------------
    // Method called to retrieve the command buffers to execute to render the current
    // frame.
    //------------------------------------------------------------------------------------
    virtual void getCommandBuffers(
        float elapsed, uint32_t imageIndex, std::vector<VkCommandBuffer>& outCommandBuffers
    ) override
    {
    }

    //------------------------------------------------------------------------------------
    // Method called right before the swap chain destruction (will happen each time the
    // window is resized and at application shutdown).
    //
    // Use it to destroy your own Vulkan objects (like the framebuffers) that depends on
    // the swap chain (ie. the dimensions and number of its images).
    //------------------------------------------------------------------------------------
    virtual void onSwapChainAboutToBeDestroyed() override
    {
    }

    //------------------------------------------------------------------------------------
    // Method called after exiting the main loop.
    //
    // Use it to destroy your own Vulkan objects (render passes, graphics pipelines,
    // framebuffers, vertex & index buffers, command buffers, ...).
    //------------------------------------------------------------------------------------
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
