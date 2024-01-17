/** knm::vulkan_tools - v1.1.0 - MIT License

DESCRIPTION

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

    The implementation is heavily based on https://vulkan-tutorial.com/. The idea is to
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

    knm::vulkan_tools is a single-file, header-only C++ library. It relies on GLFW
    (https://www.glfw.org/) to create and manage the window.

    Everything is defined in the "knm::vk" namespace ("knm" for my username, "Kanma",
    and "vk" for "Vulkan").


USAGE

    1. Copy this file in a convenient place for your project

    2. In *one* C++ file, add the following code to create the implementation:

        #define KNM_VULKAN_TOOLS_IMPLEMENTATION
        #include <knm_vulkan_tools.hpp>

    In other files, just use #include <knm_vulkan_tools.hpp>


    The most simple usage is demonstrated in the "minimal" example, reproduced here:

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


LICENSE

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

*/


#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <map>
#include <string>
#include <stdexcept>


#ifdef KNM_VULKAN_TOOLS_IMPLEMENTATION
    #include <iostream>  // Necessary for std::cerr
    #include <set>
    #include <cstring>   // Necessary for strcmp()
    #include <cstdint>   // Necessary for uint32_t
    #include <limits>    // Necessary for std::numeric_limits
    #include <algorithm> // Necessary for std::clamp()
    #include <fstream>
#endif


#define KNM_VULKAN_TOOLS_VERSION_MAJOR 1
#define KNM_VULKAN_TOOLS_VERSION_MINOR 1
#define KNM_VULKAN_TOOLS_VERSION_PATCH 0


namespace knm {
namespace vk {

    //------------------------------------------------------------------------------------
    /// @brief  Contain all the settings that can affect the behavior of the Application
    ///         class without requiring the user to override any of its methods.
    ///
    /// Change their value in the constructor of your own application class by using the
    /// 'config' attribute.
    //------------------------------------------------------------------------------------
    struct config_t
    {
        // Window settings
        uint32_t windowWidth = 800;                 ///< Window width
        uint32_t windowHeight = 600;                ///< Window height
        std::string windowTitle = "Vulkan demo";    ///< Window title

        /// Device extensions required by the application
        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        /// Validation layers to use (only in debug builds)
        std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        /// Function that the validation layers must call to output informations to the
        /// user
        PFN_vkDebugUtilsMessengerCallbackEXT debugCallback = nullptr;

        /// The highest version of Vulkan that the application is designed to use
#ifdef VK_API_VERSION_1_3
        uint32_t vulkanVersion = VK_API_VERSION_1_3;
#elif defined(VK_API_VERSION_1_2)
        uint32_t vulkanVersion = VK_API_VERSION_1_2;
#elif defined(VK_API_VERSION_1_1)
        uint32_t vulkanVersion = VK_API_VERSION_1_1;
#else
        uint32_t vulkanVersion = VK_API_VERSION_1_0;
#endif

#ifdef VK_API_VERSION_1_3
        /// Vulkan API 1.3 required features
        VkPhysicalDeviceVulkan13Features features13{};
#endif

#ifdef VK_API_VERSION_1_2
        /// Vulkan API 1.2 required features
        VkPhysicalDeviceVulkan12Features features12{};
#endif

#ifdef VK_API_VERSION_1_1
        /// Vulkan API 1.1 required features
        VkPhysicalDeviceVulkan11Features features11{};
#endif

        /// Vulkan API 1.0 required features
        VkPhysicalDeviceFeatures features10{};

        // Other settings
        std::string applicationName = "Vulkan demo";    ///< Application name
    };


    //------------------------------------------------------------------------------------
    /// @brief  Contain the indices of the queue families required by the application
    ///
    /// Note that the default implementation only support two queue families: one for the
    /// graphics and one for the presentation (which can be the same one, depending on the
    /// graphics card). Should you need more queue families, you'll need to override the
    /// Application::findQueueFamilies() method.
    //------------------------------------------------------------------------------------
    struct queueFamilyIndices_t
    {
        /// Number of required queue families
        size_t nbRequiredFamilies = 2;

        /// The queue families found on the device (a map ID -> queue_family_index)
        std::map<uint32_t, uint32_t> families;

        /// Indicates if all the required queue families were found
        inline bool isComplete() const
        {
            return families.size() == nbRequiredFamilies;
        }
    };

    /// Identifier for the queue family used for the graphics
    const uint32_t GRAPHICS_QUEUE_FAMILY = 0;

    /// Identifier for the queue family used for the presentation
    const uint32_t PRESENTATION_QUEUE_FAMILY = 1;


    //------------------------------------------------------------------------------------
    /// @brief  Contain details about the capabilities, formats and presentation modes
    ///         supported by the window surface
    //------------------------------------------------------------------------------------
    struct swapChainSupportDetails_t
    {
        /// The surface capabilities
        VkSurfaceCapabilitiesKHR capabilities;

        /// The supported formats
        std::vector<VkSurfaceFormatKHR> formats;

        /// The supported presentation modes
        std::vector<VkPresentModeKHR> presentationModes;
    };


    // Indicates if the calidation layers must be used (only in debug builds)
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


    /********************************* HELPER FUNCTIONS **********************************/

    //------------------------------------------------------------------------------------
    /// @brief  Returns the content of a binary file
    ///
    /// @param  filename    Path to the file
    ///
    /// @returns            The content of the file
    //------------------------------------------------------------------------------------
    extern std::vector<char> readFile(const std::string& filename);


    //------------------------------------------------------------------------------------
    /// @brief  Called when the framebuffer of the window is resized
    ///
    /// @param  window  The window
    /// @param  width   Width of the window
    /// @param  height  Height of the window
    //------------------------------------------------------------------------------------
    extern void onFramebufferWindowResized(GLFWwindow* window, int width, int height);


    /******************************** APPLICATION CLASS *********************************/

    //------------------------------------------------------------------------------------
    /// @brief  Handles all the setup of the window and Vulkan
    ///
    /// The user must inherit from this class, and implement a few methods to create its own
    /// Vulkan objects (render passes, graphics pipelines, framebuffers, vertex & index
    /// buffers, command buffers, ...) and do the actual rendering.
    //------------------------------------------------------------------------------------
    class Application
    {
    public:
    /// @name Public methods
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// Should you want to change any setting in the 'config' attribute, do it in the
        /// constructor of your derived application class, before calling the run()
        /// method.
        //--------------------------------------------------------------------------------
        Application();

        //--------------------------------------------------------------------------------
        /// @brief  Run the application. Doesn't return until the window is closed.
        //--------------------------------------------------------------------------------
        void run();
    /// @}


        //_____ Methods to implement __________
    protected:
    /// @name Methods to implement
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Method called after everything was initialised (window, instance,
        ///         logical device, swap chain), right before entering the main loop.
        ///
        /// Use it to create your own Vulkan objects (render passes, graphics pipelines,
        /// vertex & index buffers, command buffers, ...).
        ///
        /// Application-specific, must be implemented by the user
        //--------------------------------------------------------------------------------
        virtual void createVulkanObjects() = 0;

        //--------------------------------------------------------------------------------
        /// @brief  Method called after the swap chain was created (will happen each time
        ///         the window is resized and at application startup).
        ///
        /// Use it to create your own Vulkan objects (like the framebuffers) that depends
        /// on the swap chain (ie. the dimensions and number of its images).
        ///
        /// Application-specific, must be implemented by the user
        //--------------------------------------------------------------------------------
        virtual void onSwapChainReady() = 0;

        //--------------------------------------------------------------------------------
        /// @brief  Method called to retrieve the number of command buffers that need to be
        ///         executed to render the current frame.
        ///
        /// Application-specific, must be implemented by the user
        ///
        /// @returns    The number of command buffers
        //--------------------------------------------------------------------------------
        virtual uint32_t getNbCommandBuffers() const = 0;

        //--------------------------------------------------------------------------------
        /// @brief  Method called to retrieve the command buffers to execute to render the
        ///         current frame.
        ///
        /// The number of those command buffers must correspond to the value returned by
        /// getNbCommandBuffers().
        ///
        /// Application-specific, must be implemented by the user
        ///
        /// @param  elapsed     The time elapsed since the last call (in seconds)
        /// @param  imageIndex  Index of the swap chain image to render to
        ///
        /// @param[out]  commandBuffers     The command buffers to execute to render the
        ///                                 current frame.
        //--------------------------------------------------------------------------------
        virtual void getCommandBuffers(
            float elapsed, uint32_t imageIndex, std::vector<VkCommandBuffer>& commandBuffers
        ) = 0;

        //--------------------------------------------------------------------------------
        /// @brief  Method called right before the swap chain destruction (will happen each
        ///         time the window is resized and at application shutdown).
        ///
        /// Use it to destroy your own Vulkan objects (like the framebuffers) that depends
        /// on the swap chain (ie. the dimensions and number of its images).
        ///
        /// Application-specific, must be implemented by the user
        //--------------------------------------------------------------------------------
        virtual void onSwapChainAboutToBeDestroyed() = 0;

        //--------------------------------------------------------------------------------
        /// @brief  Method called after exiting the main loop.
        ///
        /// Use it to destroy your own Vulkan objects (render passes, graphics pipelines,
        /// framebuffers, vertex & index buffers, command buffers, ...).
        ///
        /// Application-specific, must be implemented by the user
        //--------------------------------------------------------------------------------
        virtual void destroyVulkanObjects() = 0;
    /// @}


        //_____ High-level management methods __________
    protected:
    /// @name High-level management methods
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Create the GLFW window
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void initWindow();

        //--------------------------------------------------------------------------------
        /// @brief  Initialise all the Vulkan objects (instance, logical device, swap
        ///         chain)
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void initVulkan();

        //--------------------------------------------------------------------------------
        /// @brief  Main loop, in charge of the rendering
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void mainLoop();

        //--------------------------------------------------------------------------------
        /// @brief  Method called during the main loop, each time a new frame must be
        ///         rendered.
        ///
        /// At a high level, rendering a frame in Vulkan consists of a common set of steps:
        ///   - Wait for the previous frame to finish
        ///   - Acquire an image from the swap chain
        ///   - Record a command buffer which draws the scene onto that image
        ///   - Submit the recorded command buffer
        ///   - Present the swap chain image
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        ///
        /// @param  elapsed     The time elapsed since the last call (in seconds)
        //--------------------------------------------------------------------------------
        virtual void drawFrame(float elapsed);

        //--------------------------------------------------------------------------------
        /// @brief  Destroy all the Vulkan objects (instance, logical device, swap chain)
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void cleanup();
    /// @}


        //_____ Utility methods __________
    public:
    /// @name Utility methods
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Creates a shader module from its SPIR-V binary code
        ///
        /// @param  code    The SPIR-V binary code of the shader
        ///
        /// @returns        The shader module
        //--------------------------------------------------------------------------------
        VkShaderModule createShaderModule(const std::vector<char>& code) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to create a buffer
        ///
        /// @param  size        Size of the buffer
        /// @param  usage       Usage flags
        /// @param  properties  Memory properties
        ///
        /// @param[out] buffer          The created buffer
        /// @param[out] bufferMemory    Memory associated with the buffer
        //--------------------------------------------------------------------------------
        void createBuffer(
            VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
            VkBuffer& buffer, VkDeviceMemory& bufferMemory
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to create an image
        ///
        /// @param  width       Width of the image
        /// @param  height      Height of the image
        /// @param  mipLevels   Number of mipmap levels
        /// @param  nbSamples   Number of samples to use
        /// @param  format      Image format
        /// @param  tiling      Tiling
        /// @param  usage       Usage flags
        /// @param  properties  Memory properties
        ///
        /// @param[out] image           The created image
        /// @param[out] imageMemory     Memory associated with the image
        //--------------------------------------------------------------------------------
        void createImage(
            uint32_t width, uint32_t height, uint32_t mipLevels,
            VkSampleCountFlagBits nbSamples, VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
            VkImage& image, VkDeviceMemory& imageMemory
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Creates an image view for an image
        ///
        /// @param  image       The image
        /// @param  format      Image format
        /// @param  aspectFlags Aspect flags
        /// @param  mipLevels   Number of mipmap levels
        ///
        /// @returns    The image view
        //--------------------------------------------------------------------------------
        VkImageView createImageView(
            VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
            uint32_t mipLevels
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to copy data from a buffer to another
        ///
        /// For instance from a buffer on the CPU to one on the GPU.
        ///
        /// @param  commandPool The command pool to use to create a command buffer
        /// @param  srcBuffer   The buffer to copy from
        /// @param  dstBuffer   The buffer to copy to
        /// @param  size        Size of the data to copy
        //--------------------------------------------------------------------------------
        void copyBuffer(
            VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer,
            VkDeviceSize size
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to copy data from a buffer to an image
        ///
        /// @param  commandPool The command pool to use to create a command buffer
        /// @param  buffer      The buffer to copy from
        /// @param  image       The image to copy to
        /// @param  width       Width of the image
        /// @param  height      Height of the image
        //--------------------------------------------------------------------------------
        void copyBufferToImage(
            VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width,
            uint32_t height
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to handle layout transitions for images
        ///
        /// @param  commandPool The command pool to use to create a command buffer
        /// @param  image       The image to transition
        /// @param  format      Image format
        /// @param  oldLayout   The layout to transition from
        /// @param  newLayout   The layout to transition to
        /// @param  mipLevels   Number of mipmap levels
        //--------------------------------------------------------------------------------
        void transitionImageLayout(
            VkCommandPool commandPool, VkImage image, VkFormat format,
            VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to generate the mipmaps of a texture image
        ///
        /// @param  commandPool The command pool to use to create a command buffer
        /// @param  image       The image
        /// @param  format      Image format
        /// @param  width       Width of the image
        /// @param  height      Height of the image
        /// @param  mipLevels   Number of mipmap levels
        //--------------------------------------------------------------------------------
        void generateMipmaps(
            VkCommandPool commandPool, VkImage image, VkFormat format,
            int32_t width, int32_t height, uint32_t mipLevels
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to record commands to copy data from a buffer to another
        ///
        /// For instance from a buffer on the CPU to one on the GPU.
        ///
        /// @param  commandBuffer   The command buffer to record our commands to
        /// @param  srcBuffer       The buffer to copy from
        /// @param  dstBuffer       The buffer to copy to
        /// @param  size            Size of the data to copy
        //--------------------------------------------------------------------------------
        void recordCopyBufferCommand(
            VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
            VkDeviceSize size
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to record commands to copy data from a buffer to an image
        ///
        /// @param  commandBuffer   The command buffer to record our commands to
        /// @param  buffer          The buffer to copy from
        /// @param  image           The image to copy to
        /// @param  width           Width of the image
        /// @param  height          Height of the image
        //--------------------------------------------------------------------------------
        void recordCopyBufferToImageCommand(
            VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width,
            uint32_t height
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to record commands to handle layout transitions for
        ///         images
        ///
        /// @param  commandBuffer   The command buffer to record our commands to
        /// @param  image           The image to transition
        /// @param  format          Image format
        /// @param  oldLayout       The layout to transition from
        /// @param  newLayout       The layout to transition to
        /// @param  mipLevels       Number of mipmap levels
        //--------------------------------------------------------------------------------
        void recordTransitionImageLayoutCommand(
            VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
            VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to record commands to generate the mipmaps of a texture
        ///         image
        ///
        /// @param  commandBuffer   The command buffer to record our commands to
        /// @param  image           The image
        /// @param  format          Image format
        /// @param  width           Width of the image
        /// @param  height          Height of the image
        /// @param  mipLevels       Number of mipmap levels
        //--------------------------------------------------------------------------------
        void recordGenerateMipmapsCommand(
            VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
            int32_t width, int32_t height, uint32_t mipLevels
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to create a command buffer destined to only be executed once.
        ///
        /// Call endSingleTimeCommands() once you have added your commands to the buffer
        /// to execute them.
        ///
        /// @param  commandPool     The command pool to use to create the command buffer
        ///
        /// @returns                The command buffer
        //--------------------------------------------------------------------------------
        VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool) const;

        //--------------------------------------------------------------------------------
        /// @brief  Helper method to execute and release a command buffer destined to only
        ///         be executed once (created with beginSingleTimeCommands())
        ///
        /// @param  commandPool     The command pool used to create the command buffer
        /// @param  commandBuffer   The command buffer to execute
        //--------------------------------------------------------------------------------
        void endSingleTimeCommands(
            VkCommandPool commandPool, VkCommandBuffer commandBuffer
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the index of a memory on the physical device (graphics card)
        ///         supporting the provided properties
        ///
        /// @param  typeFilter  Type of memory
        /// @param  properties  Properties of the desired memory
        ///
        /// @returns            Index of the memory
        //--------------------------------------------------------------------------------
        uint32_t findMemoryType(
            uint32_t typeFilter, VkMemoryPropertyFlags properties
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the first image format supported by the physical device
        ///         (graphics card) corresponding to the provided criteria
        ///
        /// @param  candidates  Candidate image formats (in order of preference)
        /// @param  tiling      Desired tiling of the image
        /// @param  features    Desired features
        ///
        /// @returns            The format
        //--------------------------------------------------------------------------------
        VkFormat findSupportedFormat(
            const std::vector<VkFormat>& candidates, VkImageTiling tiling,
            VkFormatFeatureFlags features
        ) const;
    /// @}


        //_____ Instance-related methods __________
    protected:
    /// @name Instance-related methods
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Initialize the Vulkan library by creating an instance. The instance is
        ///         the connection between the application and the Vulkan library and
        ///         creating it involves specifying some details about the application to
        ///         the driver.
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void createInstance();

        //--------------------------------------------------------------------------------
        /// @brief  Creates the surface to use to render to the GLFW window
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void createSurface();

        //--------------------------------------------------------------------------------
        /// @brief  Returns the required list of extensions
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        ///
        /// @returns    The list of extension
        //--------------------------------------------------------------------------------
        virtual std::vector<const char*> getRequiredExtensions() const;
    /// @}


        //_____ Physical devices __________
    protected:
    /// @name Physical devices
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Select a graphics card in the system that supports the features we
        ///         need. Use the first graphics card that suits our needs.
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void pickPhysicalDevice();

        //--------------------------------------------------------------------------------
        /// @brief  Indicates if a physical device/graphics card is suitable for our needs
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual bool isDeviceSuitable(VkPhysicalDevice device) const;

        //--------------------------------------------------------------------------------
        /// @brief  Search suitable queue families on a given physical device
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual queueFamilyIndices_t findQueueFamilies(VkPhysicalDevice device) const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the required list of device extensions
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual std::vector<const char*> getRequiredDeviceExtensions(
            VkPhysicalDevice device
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Indicates if a physical device/graphics card supports the device
        ///         extensions we need
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the maximum level of MSAA (multisample anti-aliasing)
        ///         supported by the physical device
        //--------------------------------------------------------------------------------
        VkSampleCountFlagBits getMaxUsableSampleCount() const;
    /// @}


        //_____ Logical devices __________
    protected:
    /// @name Logical devices
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Setup a logical device to interface with a physical device, using
        ///         queues
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void createLogicalDevice();
    /// @}


        //_____ Swap chain __________
    protected:
    /// @name Swap chain
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Create the swap chain
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void createSwapChain();

        //--------------------------------------------------------------------------------
        /// @brief  Destroy all the objects related to the swap chain
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void cleanupSwapChain();

        //--------------------------------------------------------------------------------
        /// @brief  This method must called each time the window is resized, to recreate
        ///         the swap chain and objects that depend on it or the window size
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void recreateSwapChain();

        //--------------------------------------------------------------------------------
        /// @brief  Create image views to access the images of the swap chain
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        //--------------------------------------------------------------------------------
        virtual void createImageViews();

        //------------------------------------------------------------------------------------
        /// @brief  Creates the semaphores and fences that will be used to synchronise the
        ///         rendering (one set for each frame-in-flight)
        //------------------------------------------------------------------------------------
        virtual void createSyncObjects();

        //--------------------------------------------------------------------------------
        /// @brief  Retrieve the details about the swap chain support by a physical device
        ///
        /// @param  device  The physical device
        ///
        /// @returns        The details about the swap chain support
        //--------------------------------------------------------------------------------
        swapChainSupportDetails_t querySwapChainSupport(VkPhysicalDevice device) const;

        //--------------------------------------------------------------------------------
        /// @brief  Select the best surface format from the provided list
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        ///
        /// @param  availableFormats    List of available formats
        ///
        /// @returns                    The best surface format
        //--------------------------------------------------------------------------------
        virtual VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Select the best presentation mode from the provided list
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        ///
        /// @param  availablePresentModes    List of available modes
        ///
        /// @returns                        The best presentation mode
        //--------------------------------------------------------------------------------
        virtual VkPresentModeKHR chooseSwapPresentationMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes
        ) const;
        
        //--------------------------------------------------------------------------------
        /// @brief  Select the swap extent (the resolution of the swap chain images)
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        ///
        /// @param  capabilities    The capabilities of the surface
        ///
        /// @returns                The selected swap extent
        //--------------------------------------------------------------------------------
        virtual VkExtent2D chooseSwapExtent(
            const VkSurfaceCapabilitiesKHR& capabilities
        ) const;
    /// @}


        //_____ Validation layers __________
    protected:
    /// @name Validation layers
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Checks if all of the requested validation layers are available
        //--------------------------------------------------------------------------------
        bool checkValidationLayerSupport() const;

        //--------------------------------------------------------------------------------
        /// @brief  Populate the provided messenger creation info
        ///
        /// Can be overriden by the user if the default implementation doesn't fulfill its
        /// needs.
        ///
        /// @param[out] createInfo  The structure to populate
        //--------------------------------------------------------------------------------
        virtual void populateDebugMessengerCreateInfo(
            VkDebugUtilsMessengerCreateInfoEXT& createInfo
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Tell Vulkan about the callback function to call from validation layers
        //--------------------------------------------------------------------------------
        void setupDebugMessenger();
    /// @}


        //_____ Attributes __________
    protected:
        // The configuration of the application
        config_t config;

        // Window, instance and surface
        GLFWwindow* window = nullptr;
        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkFormat surfaceImageFormat = VK_FORMAT_UNDEFINED;
        bool framebufferResized = false;

        // Devices
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkSampleCountFlagBits msaaNbMaxSamples = VK_SAMPLE_COUNT_1_BIT;

        // Queues
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentationQueue = VK_NULL_HANDLE;

        // Swap chain
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        VkExtent2D swapChainExtent;

        // Rendering-related synchronisation objects
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        uint32_t currentFrame = 0;
        std::vector<VkCommandBuffer> commandBufferList;

        // Debug messenger (when validation layers are used)
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;


        //_____ Constants __________
    protected:
        const int MAX_NB_FRAMES_IN_FLIGHT = 2;


        //_____ Friend functions __________
        friend void onFramebufferWindowResized(GLFWwindow* window, int width, int height);
    };


#ifdef KNM_VULKAN_TOOLS_IMPLEMENTATION

    /***************************** DEBUG CALLBACK FUNCTION ******************************/

    //------------------------------------------------------------------------------------
    // The default callback function to call from validation layers
    //------------------------------------------------------------------------------------
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    )
    {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }


    /********************************* HELPER FUNCTIONS **********************************/

    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("Failed to open file!");

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    //-----------------------------------------------------------------------

    void onFramebufferWindowResized(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    //-----------------------------------------------------------------------

    bool compareFeatures(
        void* required, void* supported, size_t size, size_t offset, bool apiVersionSupported
    )
    {
        VkBool32* required_bool = (VkBool32*) (static_cast<uint8_t*>(required) + offset);
        VkBool32* supported_bool = (VkBool32*) (static_cast<uint8_t*>(supported) + offset);
        size_t nb = (size - offset) / sizeof(VkBool32);

        for (size_t i = 0; i < nb; ++i)
        {
            if (required_bool[i] && (!supported_bool[i] || !apiVersionSupported))
                return false;
        }

        return true;
    }


    /*************************** CONSTRUCTION / DESTRUCTION *****************************/

    Application::Application()
    {
        config.debugCallback = debugCallback;

#ifdef VK_API_VERSION_1_3
        config.features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        config.features12.pNext = &config.features13;
#endif

#ifdef VK_API_VERSION_1_2
        config.features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        config.features11.pNext = &config.features12;
#endif

#ifdef VK_API_VERSION_1_1
        config.features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
#endif

        config.features10.samplerAnisotropy = VK_TRUE;
    }

    //-----------------------------------------------------------------------

    void Application::run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

    //-----------------------------------------------------------------------

    void Application::initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(
            config.windowWidth, config.windowHeight, config.windowTitle.data(),
            nullptr, nullptr
        );

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, onFramebufferWindowResized);
    }

    //-----------------------------------------------------------------------

    void Application::initVulkan()
    {
        createInstance();
        setupDebugMessenger();
        createSurface();

        pickPhysicalDevice();
        createLogicalDevice();
        createSyncObjects();

        createVulkanObjects();

        createSwapChain();
        createImageViews();
        onSwapChainReady();
    }

    //-----------------------------------------------------------------------

    void Application::mainLoop()
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        auto previousTime = startTime;

        while (!glfwWindowShouldClose(window)) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float elapsed = std::chrono::duration<float, std::chrono::seconds::period>(
                currentTime - previousTime
            ).count();

            // Retrieve window-related events
            glfwPollEvents();

            // Ensure we have allocated enough space for the user-supplied
            // command buffers
            uint32_t nbCommandBuffers = getNbCommandBuffers();
            if (nbCommandBuffers != commandBufferList.size())
                commandBufferList.resize(nbCommandBuffers);

            // Draw the frame (if necessary)
            if (nbCommandBuffers > 0)
                drawFrame(elapsed);

            previousTime = currentTime;
        }

        vkDeviceWaitIdle(device);
    }

    //-----------------------------------------------------------------------

    void Application::drawFrame(float elapsed)
    {
        // Wait for the previous frame to finish
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        // Acquire an image from the swap chain
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE, &imageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        // Ask the user code to do some rendering
        getCommandBuffers(elapsed, imageIndex, commandBufferList);

        // Submit the command buffer
        VkSemaphore waitSemaphores[] = {
            imageAvailableSemaphores[currentFrame]
        };

        VkSemaphore signalSemaphores[] = {
            renderFinishedSemaphores[currentFrame]
        };

        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = commandBufferList.size();
        submitInfo.pCommandBuffers = commandBufferList.data();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit draw command buffer!");

        // Presentation
        VkSwapchainKHR swapChains[] = {swapChain};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(presentationQueue, &presentInfo);

        // Handle errors from the frame presentation on screen
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR) ||
            framebufferResized)
        {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_NB_FRAMES_IN_FLIGHT;
    }

    //-----------------------------------------------------------------------

    void Application::cleanup()
    {
        cleanupSwapChain();

        destroyVulkanObjects();

        for (size_t i = 0; i < imageAvailableSemaphores.size(); ++i)
        {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers)
        {
            // Destroy the debug messenger (we have to look up the address of the function
            // ourselves)
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
                instance, "vkDestroyDebugUtilsMessengerEXT"
            );

            if (func != nullptr)
                func(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    //-----------------------------------------------------------------------

    VkShaderModule Application::createShaderModule(const std::vector<char>& code) const
    {
        // Fill in a struct with some information about the shader
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module!");

        return shaderModule;
    }

    //-----------------------------------------------------------------------

    void Application::createBuffer(
        VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory
    ) const
    {
        // Create the buffer
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create buffer!");

        // Allocate some memory for it
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate buffer memory!");

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    //-----------------------------------------------------------------------

    void Application::createImage(
        uint32_t width, uint32_t height, uint32_t mipLevels,
        VkSampleCountFlagBits nbSamples, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory
    ) const
    {
        // Create the image
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = nbSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
            throw std::runtime_error("Failed to create an image!");

        // Allocate memory on the GPU for the image
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate image memory!");

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    //-----------------------------------------------------------------------

    VkImageView Application::createImageView(
        VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels
    ) const
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image view!");

        return imageView;
    }

    //-----------------------------------------------------------------------

    void Application::copyBuffer(
        VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size
    ) const
    {
        // Create the command buffer
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

        // Copy command
        recordCopyBufferCommand(commandBuffer, srcBuffer, dstBuffer, size);

        // Execute and release the command buffer
        endSingleTimeCommands(commandPool, commandBuffer);
    }

    //-----------------------------------------------------------------------

    void Application::copyBufferToImage(
        VkCommandPool commandPool, VkBuffer buffer, VkImage image,
        uint32_t width, uint32_t height
    ) const
    {
        // Create the command buffer
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

        // Image copy command
        recordCopyBufferToImageCommand(commandBuffer, buffer, image, width,  height);

        // Execute and release the command buffer
        endSingleTimeCommands(commandPool, commandBuffer);
    }

    //-----------------------------------------------------------------------

    void Application::transitionImageLayout(
        VkCommandPool commandPool, VkImage image, VkFormat format,
        VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels
    ) const
    {
        // Create the command buffer
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

        // Create a barrier to perform layout transition
        recordTransitionImageLayoutCommand(
            commandBuffer, image, format, oldLayout, newLayout, mipLevels
        );

        // Execute and release the command buffer
        endSingleTimeCommands(commandPool, commandBuffer);
    }

    //-----------------------------------------------------------------------

    void Application::generateMipmaps(
        VkCommandPool commandPool, VkImage image, VkFormat imageFormat,
        int32_t texWidth, int32_t texHeight, uint32_t mipLevels
    ) const
    {
        // Create the command buffer
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

        // Create a barrier to perform layout transition of each mipmap level
        recordGenerateMipmapsCommand(
            commandBuffer, image, imageFormat, texWidth, texHeight,  mipLevels
        );

        // Execute and release the command buffer
        endSingleTimeCommands(commandPool, commandBuffer);
    }

    //-----------------------------------------------------------------------

    void Application::recordCopyBufferCommand(
        VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size
    ) const
    {
        // Copy command
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    }

    //-----------------------------------------------------------------------

    void Application::recordCopyBufferToImageCommand(
        VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image,
        uint32_t width, uint32_t height
    ) const
    {
        // Image copy command
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    }

    //-----------------------------------------------------------------------

    void Application::recordTransitionImageLayoutCommand(
        VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
        VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels
    ) const
    {
        // Create a barrier to perform layout transition
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) &&
            (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) &&
                 (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    //-----------------------------------------------------------------------

    void Application::recordGenerateMipmapsCommand(
        VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat,
        int32_t texWidth, int32_t texHeight, uint32_t mipLevels
    ) const
    {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
            throw std::runtime_error("Texture image format does not support linear blitting!");

        // Create a barrier to perform layout transition of each mipmap level
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        // Iterate over all mipmap level
        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; ++i)
        {
            // Transition the layout of level i-1 to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            // Copy and resize the image of level i-1 to level i
            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(
                commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR
            );

            // Transition the layout of level i-1 to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        // Transition the layout of the last level to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    //-----------------------------------------------------------------------

    VkCommandBuffer Application::beginSingleTimeCommands(VkCommandPool commandPool) const
    {
        // Allocate a command buffer
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        // Start recording commands
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    //-----------------------------------------------------------------------

    void Application::endSingleTimeCommands(
        VkCommandPool commandPool, VkCommandBuffer commandBuffer
    ) const
    {
        // End recording commands
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Execute the command buffer
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        // Cleanup
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    //-----------------------------------------------------------------------

    uint32_t Application::findMemoryType(
        uint32_t typeFilter, VkMemoryPropertyFlags properties
    ) const
    {
        // Query info about the available types of memory
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        // Find a memory type that is suitable for the buffer
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) &&
                ((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    //-----------------------------------------------------------------------

    VkFormat Application::findSupportedFormat(
        const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features
    ) const
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if ((tiling == VK_IMAGE_TILING_LINEAR) &&
                ((props.linearTilingFeatures & features) == features))
            {
                return format;
            }
            else if ((tiling == VK_IMAGE_TILING_OPTIMAL) &&
                     ((props.optimalTilingFeatures & features) == features))
            {
                return format;
            }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    //-----------------------------------------------------------------------

    void Application::createInstance()
    {
        // If necessary, check that all the needed validation layers are available
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        // Fill in a struct with some information about our application
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = config.applicationName.data();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = config.vulkanVersion;

        // Fill in a struct with sufficient information for creating an instance
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        //-- Enable the needed extensions
        auto requiredExtensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifdef __APPLE__
        //-- MacOS-related flags
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        //-- Validation layers
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(config.validationLayers.size());
            createInfo.ppEnabledLayerNames = config.validationLayers.data();

            // Needed to have a debug messenger during instance creation/destruction
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        // Creation of the instance
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create instance!");
        }
    }

    //-----------------------------------------------------------------------

    void Application::createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface!");
    }

    //-----------------------------------------------------------------------

    std::vector<const char*> Application::getRequiredExtensions() const
    {
        std::vector<const char*> extensions;

        // Retrieve GLFW-related extensions
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for(uint32_t i = 0; i < glfwExtensionCount; ++i)
            extensions.emplace_back(glfwExtensions[i]);

#ifdef __APPLE__
        // MacOS-related extensions
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

        // Validation layers-related extensions
        if (enableValidationLayers) {
            extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Retrieve the list of supported extensions
        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
        
        // Check for the availability of the "VK_KHR_get_physical_device_properties2"
        // extension (needed by the "VK_KHR_portability_subset" device extension, see
        // checkDeviceExtensionSupport())
        for (const auto& extension : availableExtensions)
        {
            if (strcmp(extension.extensionName, "VK_KHR_get_physical_device_properties2") == 0)
            {
                extensions.emplace_back("VK_KHR_get_physical_device_properties2");
                break;
            }
        }

        return extensions;
    }


    /******************************** PHYSICAL DEVICES **********************************/

    void Application::pickPhysicalDevice()
    {
        // Retrieve the number of physical devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");

        // Retrieve a list of all physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Find the first suitable device
        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                msaaNbMaxSamples = getMaxUsableSampleCount();
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("Failed to find a suitable GPU!");

        // Retrieve and store the best surface format supported by the physical device for later use
        swapChainSupportDetails_t swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
            swapChainSupport.formats
        );

        surfaceImageFormat = surfaceFormat.format;
    }

    //-----------------------------------------------------------------------

    bool Application::isDeviceSuitable(VkPhysicalDevice device) const
    {
        // Retrieve the indices of the queue families we need from the physical
        // device
        queueFamilyIndices_t indices = findQueueFamilies(device);

        // Check that the device extensions we need are supported
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        // Check that the swap chain support is adequate on the device (we need
        // at least one surface format and one presentation mode)
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            swapChainSupportDetails_t swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() &&
                                !swapChainSupport.presentationModes.empty();
        }

        // First check to dismiss inadequate devices
        if (!indices.isComplete() || !extensionsSupported || !swapChainAdequate)
            return false;

        // Check that the required (api-specific) features are supported
        if (config.vulkanVersion == VK_API_VERSION_1_0)
        {
            // Retrieve the features supported by the device
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            // Check if all the required features are there
            return compareFeatures(
                (void*) &config.features10, (void*) &supportedFeatures,
                sizeof(VkPhysicalDeviceFeatures), 0, true
            );
        }
        else
        {
            // Retrieve the properties of the device
            VkPhysicalDeviceProperties physicalDeviceProperties;
            vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

            // Retrieve the features supported by the device
            VkPhysicalDeviceFeatures2 supportedFeatures{};
            supportedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

#ifdef VK_API_VERSION_1_1
            VkPhysicalDeviceVulkan11Features features11{};
            features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
            supportedFeatures.pNext = &features11;
#endif

#ifdef VK_API_VERSION_1_2
            VkPhysicalDeviceVulkan12Features features12{};
            features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            features11.pNext = &features12;
#endif

#ifdef VK_API_VERSION_1_3
            VkPhysicalDeviceVulkan13Features features13{};
            features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
            features12.pNext = &features13;
#endif

            vkGetPhysicalDeviceFeatures2(device, &supportedFeatures);

            // Check if all the required 1.0 features are there
            if (!compareFeatures(
                    (void*) &config.features10,
                    (void*) &supportedFeatures.features,
                    sizeof(VkPhysicalDeviceFeatures), 0, true
            ))
            {
                return false;
            }

#ifdef VK_API_VERSION_1_1
            // Check if all the required 1.1 features are there
            if (!compareFeatures(
                    (void*) &config.features11,
                    (void*) &features11,
                    sizeof(VkPhysicalDeviceVulkan11Features),
                    offsetof(VkPhysicalDeviceVulkan11Features, storageBuffer16BitAccess),
                    physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_1
            ))
            {
                return false;
            }
#endif

#ifdef VK_API_VERSION_1_2
            // Check if all the required 1.2 features are there
            if (!compareFeatures(
                    (void*) &config.features12,
                    (void*) &features12,
                    sizeof(VkPhysicalDeviceVulkan12Features),
                    offsetof(VkPhysicalDeviceVulkan12Features, samplerMirrorClampToEdge),
                    physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_2
            ))
            {
                return false;
            }
#endif

#ifdef VK_API_VERSION_1_3
            // Check if all the required 1.3 features are there
            if (!compareFeatures(
                    (void*) &config.features13,
                    (void*) &features13,
                    sizeof(VkPhysicalDeviceVulkan13Features),
                    offsetof(VkPhysicalDeviceVulkan13Features, robustImageAccess),
                    physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_3
            ))
            {
                return false;
            }
#endif
        }

        return true;
    }

    //-----------------------------------------------------------------------

    queueFamilyIndices_t Application::findQueueFamilies(VkPhysicalDevice device) const
    {
        queueFamilyIndices_t indices;
        indices.nbRequiredFamilies = 2;

        // Retrieve the available queue families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            device, &queueFamilyCount, queueFamilies.data()
        );

        // Iterate through all the queue families
        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            // Are graphics commands supported?
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.families[GRAPHICS_QUEUE_FAMILY] = i;

            // Is presentation supported?
            VkBool32 presentationSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
            if (presentationSupport)
                indices.families[PRESENTATION_QUEUE_FAMILY] = i;

            // Do we have everything we need?
            if (indices.isComplete())
                break;

            ++i;
        }

        return indices;
    }

    //-----------------------------------------------------------------------

    std::vector<const char*> Application::getRequiredDeviceExtensions(VkPhysicalDevice device) const
    {
        std::vector<const char*> extensions(config.deviceExtensions.begin(), config.deviceExtensions.end());

        // Retrieve the list of supported device extensions
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // The Vulkan spec states: If the VK_KHR_portability_subset extension is
        // supported, it must be included
        for (const auto& extension : availableExtensions)
        {
            if (strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0)
            {
                extensions.emplace_back("VK_KHR_portability_subset");
                break;
            }
        }

        return extensions;
    }

    //-----------------------------------------------------------------------

    bool Application::checkDeviceExtensionSupport(VkPhysicalDevice device) const
    {
        // Retrieve the list of supported device extensions
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // Check if all the ones we require are present
        std::vector<const char*> requiredExtensions = getRequiredDeviceExtensions(device);
        std::set<std::string> deviceExtensions(requiredExtensions.begin(), requiredExtensions.end());
        for (const auto& extension : availableExtensions)
            deviceExtensions.erase(extension.extensionName);

        return deviceExtensions.empty();
    }

    //-----------------------------------------------------------------------

    VkSampleCountFlagBits Application::getMaxUsableSampleCount() const
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts =
            physicalDeviceProperties.limits.framebufferColorSampleCounts &
            physicalDeviceProperties.limits.framebufferDepthSampleCounts;

        for (int flag = VK_SAMPLE_COUNT_64_BIT; flag > VK_SAMPLE_COUNT_1_BIT; flag >>= 1)
        {
            if (counts & flag)
                return static_cast<VkSampleCountFlagBits>(flag);
        }

        return VK_SAMPLE_COUNT_1_BIT;
    }


    /********************************* LOGICAL DEVICES **********************************/

    void Application::createLogicalDevice()
    {
        // Retrieve the indices of the queue families we need from the physical device
        queueFamilyIndices_t indices = findQueueFamilies(physicalDevice);

        // Retrieve the required device extensions
        auto requiredExtensions = getRequiredDeviceExtensions(physicalDevice);

        // Fill structs to request the creation of the queues we need
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            indices.families[GRAPHICS_QUEUE_FAMILY],
            indices.families[PRESENTATION_QUEUE_FAMILY]
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Fill a struct to create the logical device, using the above structs
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

#ifdef VK_API_VERSION_1_1
        VkPhysicalDeviceFeatures2 enabledFeatures{};
        enabledFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        enabledFeatures.pNext = &config.features11;
        memcpy(&enabledFeatures.features, &config.features10, sizeof(VkPhysicalDeviceFeatures));

        createInfo.pNext = &enabledFeatures;
#else
        createInfo.pEnabledFeatures = &config.features10;
#endif

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        if (enableValidationLayers) {
            // Not necessarya anymore on current implementations of Vulkan, only
            // here for backward compatibility
            createInfo.enabledLayerCount = static_cast<uint32_t>(config.validationLayers.size());
            createInfo.ppEnabledLayerNames = config.validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        // Create the logical device
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device!");

        // Creates the queue for graphics command
        vkGetDeviceQueue(device, indices.families[GRAPHICS_QUEUE_FAMILY], 0, &graphicsQueue);

        // Creates the queue for presentation
        vkGetDeviceQueue(device, indices.families[PRESENTATION_QUEUE_FAMILY], 0, &presentationQueue);
    }


    /************************************ SWAP CHAIN ************************************/

    void Application::createSwapChain()
    {
        // Choose the parameters of the swap chain (surface format, presentation mode,
        // extent)
        swapChainSupportDetails_t swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
            swapChainSupport.formats
        );

        VkPresentModeKHR presentationMode = chooseSwapPresentationMode(
            swapChainSupport.presentationModes
        );

        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        // Determine the number of images in the swap chain
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if ((swapChainSupport.capabilities.maxImageCount > 0) &&
            (imageCount > swapChainSupport.capabilities.maxImageCount))
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // Fill a struct with the creation infos of the swap chain
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentationMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        //-- How to handle swap chain images that will be used across multiple
        //   queue families (if necessary)
        queueFamilyIndices_t indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {
            indices.families[GRAPHICS_QUEUE_FAMILY],
            indices.families[PRESENTATION_QUEUE_FAMILY]
        };

        if (indices.families[GRAPHICS_QUEUE_FAMILY] != indices.families[PRESENTATION_QUEUE_FAMILY])
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        // Create the swap chain
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swap chain!");

        // Retrieve the images of the swap chain
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        // Store the extent for later use
        swapChainExtent = extent;
    }

    //-----------------------------------------------------------------------

    void Application::recreateSwapChain()
    {
        // If the window is minimzed, wait for it to be visible again
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while ((width == 0) || (height == 0))
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        // Wait for the completion of outstanding queue operations for all queues on the device
        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        onSwapChainReady();
    }

    //-----------------------------------------------------------------------

    void Application::cleanupSwapChain()
    {
        onSwapChainAboutToBeDestroyed();

        for (auto imageView : swapChainImageViews)
            vkDestroyImageView(device, imageView, nullptr);

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    //-----------------------------------------------------------------------

    void Application::createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            swapChainImageViews[i] = createImageView(
                swapChainImages[i], surfaceImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1
            );
        }
    }

    //-----------------------------------------------------------------------

    swapChainSupportDetails_t Application::querySwapChainSupport(VkPhysicalDevice device) const
    {
        swapChainSupportDetails_t details;

        // Retrieve the surface capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // Retrieve the supported surface formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                device, surface, &formatCount, details.formats.data()
            );
        }

        // Retrieve the supported presentation modes
        uint32_t presentationModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &presentationModeCount, nullptr
        );

        if (presentationModeCount != 0) {
            details.presentationModes.resize(presentationModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device, surface, &presentationModeCount, details.presentationModes.data()
            );
        }

        return details;
    }

    //-----------------------------------------------------------------------

    VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats
    ) const
    {
        // Prefer VK_FORMAT_B8G8R8A8_SRGB if it is available
        for (const auto& availableFormat : availableFormats) {
            if ((availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB) &&
                (availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
            {
                return availableFormat;
            }
        }

        // Otherwise use the first one of the list
        return availableFormats[0];
    }

    //-----------------------------------------------------------------------

    VkPresentModeKHR Application::chooseSwapPresentationMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes
    ) const
    {
        // Prefer VK_PRESENT_MODE_MAILBOX_KHR if it is available
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        // Otherwise use VK_PRESENT_MODE_FIFO_KHR (which is guaranteed to be
        // available)
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    //-----------------------------------------------------------------------

    VkExtent2D Application::chooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& capabilities
    ) const
    {
        // Is Vulkan asking for our help here (ie. on a Retina display)?
        // (see https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain#page_Swap-extent)
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        // Retrieve the actual window size in pixels (might be different from
        // the one in screen coordinates)
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // ensure the dimensions are within supported bounds
        actualExtent.width = std::clamp(
            actualExtent.width, capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );

        actualExtent.height = std::clamp(
            actualExtent.height, capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );

        return actualExtent;
    }

    //-----------------------------------------------------------------------

    void Application::createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_NB_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_NB_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_NB_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_NB_FRAMES_IN_FLIGHT; ++i)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
            }
        }
    }


    /******************************** VALIDATION LAYERS *********************************/

    bool Application::checkValidationLayerSupport() const
    {
        // First list all of the available layers
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // Next, check if all of the requested layers exist in the available layers list
        for (const char* layerName : config.validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    //-----------------------------------------------------------------------

    void Application::populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo
    ) const
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = config.debugCallback;
        createInfo.pUserData = nullptr;
    }

    //-----------------------------------------------------------------------

    void Application::setupDebugMessenger()
    {
        // Check that there is something to do
        if (!enableValidationLayers || !config.debugCallback)
            return;

        // Fill in a structure with details about the messenger and its callback
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        // Create the messenger (since it is an extension object, we have to look up the
        // address of the function to call ourselves)
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT"
        );

        if (!func || (func(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS))
            throw std::runtime_error("Failed to set up debug messenger!");
    }

#endif // KNM_VULKAN_TOOLS_IMPLEMENTATION

}
}
