/*
SPDX-FileCopyrightText: 2023 Philip Abbet

SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>

SPDX-License-Identifier: MIT
*/

#pragma once

#include <knm_vulkan_tools.hpp>
#include <string>


struct texture_t
{
    // The image containing the texture
    VkImage image;

    // The device memory allocated for the image
    VkDeviceMemory memory;

    // The view used to access the content of the image
    VkImageView view;

    // The sampler used in the shaders to access the content of the image
    VkSampler sampler;

    // Dimensions
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels;
};



//------------------------------------------------------------------------------------
// Create a texture from an image file
//------------------------------------------------------------------------------------
void createTexture(
    const knm::vk::Application* app, VkDevice device, VkCommandPool commandPool,
    const std::string& filename, float maxAnisotropy, texture_t& texture
);


//------------------------------------------------------------------------------------
// Destroy the resources used by a texture
//------------------------------------------------------------------------------------
void destroyTexture(VkDevice device, const texture_t& texture);
