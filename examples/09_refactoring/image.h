/*
SPDX-FileCopyrightText: 2023 Philip Abbet

SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>

SPDX-License-Identifier: MIT
*/

#pragma once

#include <knm_vulkan_tools.hpp>


struct image_t
{
    // The Vulkan image
    VkImage image;

    // The device memory allocated for the image
    VkDeviceMemory memory;

    // The view used to access the content of the image
    VkImageView view;

    // Dimensions
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels;
    VkSampleCountFlagBits nbSamples;
};



//------------------------------------------------------------------------------------
// Create an image
//------------------------------------------------------------------------------------
void createImage(
    const knm::vk::Application* app,
    uint32_t width, uint32_t height, uint32_t mipLevels,
    VkSampleCountFlagBits nbSamples, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImageAspectFlags aspectFlags, image_t& image
);


//------------------------------------------------------------------------------------
// Destroy the resources used by a texture
//------------------------------------------------------------------------------------
void destroyImage(VkDevice device, const image_t& image);
