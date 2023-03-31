/*
SPDX-FileCopyrightText: 2023 Philip Abbet

SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>

SPDX-License-Identifier: MIT
*/

#include "texture.h"

#include <stb_image.h>

#include <cmath>

using namespace knm::vk;


/********************************* INTERNAL FUNCTIONS ***********************************/

//----------------------------------------------------------------------------------------
// Load an image and upload it into a Vulkan image object to be used as a texture
//----------------------------------------------------------------------------------------
void createTextureImage(
    const knm::vk::Application* app, VkDevice device, VkCommandPool commandPool,
    const std::string& filename, texture_t& texture
)
{
    // Load the texture
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(
        filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha
    );

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
        throw std::runtime_error("Failed to load texture image!");

    texture.width = static_cast<uint32_t>(texWidth);
    texture.height = static_cast<uint32_t>(texHeight);

    // Compute the number of mipmap level
    texture.mipLevels = static_cast<uint32_t>(
        std::floor(std::log2(std::max(texWidth, texHeight)))
    ) + 1;

    // Create a staging buffer (usable on the CPU side)
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    app->createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    // Copy the image pixels to the staging buffer
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t) imageSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Release the memory used by the image
    stbi_image_free(pixels);

    // Create an image
    app->createImage(
        texture.width,
        texture.height,
        texture.mipLevels,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texture.image,
        texture.memory
    );

    // Create a command buffer
    VkCommandBuffer commandBuffer = app->beginSingleTimeCommands(commandPool);

    // Transition the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    app->recordTransitionImageLayoutCommand(
        commandBuffer,
        texture.image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        texture.mipLevels
    );

    // Execute the buffer to image copy operation
    app->recordCopyBufferToImageCommand(
        commandBuffer,
        stagingBuffer,
        texture.image,
        texture.width,
        texture.height
    );

    // Generate the mipmap levels
    app->recordGenerateMipmapsCommand(
        commandBuffer,
        texture.image,
        VK_FORMAT_R8G8B8A8_SRGB,
        texture.width,
        texture.height,
        texture.mipLevels
    );

    // Execute and release the command buffer
    app->endSingleTimeCommands(commandPool, commandBuffer);

    // Cleanup of the staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}


//----------------------------------------------------------------------------------------
// Creates the sampler to access the texture from shaders
//----------------------------------------------------------------------------------------
void createTextureSampler(VkDevice device, float maxAnisotropy, texture_t& texture)
{
    // Create the texture sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = maxAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(texture.mipLevels);

    if (vkCreateSampler(device, &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture sampler!");
}


/********************************** PUBLIC FUNCTIONS ************************************/

void createTexture(
    const knm::vk::Application* app, VkDevice device, VkCommandPool commandPool,
    const std::string& filename, float maxAnisotropy, texture_t& texture
)
{
    createTextureImage(app, device, commandPool, filename, texture);

    texture.view = app->createImageView(
        texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture.mipLevels
    );

    createTextureSampler(device, maxAnisotropy, texture);
}

//------------------------------------------------------------------------------

void destroyTexture(VkDevice device, const texture_t& texture)
{
    vkDestroySampler(device, texture.sampler, nullptr);
    vkDestroyImageView(device, texture.view, nullptr);

    vkDestroyImage(device, texture.image, nullptr);
    vkFreeMemory(device, texture.memory, nullptr);
}
