/*
SPDX-FileCopyrightText: 2023 Philip Abbet

SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>

SPDX-License-Identifier: MIT
*/

#include "image.h"

using namespace knm::vk;


void createImage(
    const knm::vk::Application* app,
    uint32_t width, uint32_t height, uint32_t mipLevels,
    VkSampleCountFlagBits nbSamples, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImageAspectFlags aspectFlags, image_t& image
)
{
    app->createImage(
        width, height, mipLevels, nbSamples,
        format, tiling, usage, properties,
        image.image, image.memory
    );

    image.view = app->createImageView(
        image.image, format, aspectFlags, mipLevels
    );

    image.width = width;
    image.height = height;
    image.mipLevels = mipLevels;
    image.nbSamples = nbSamples;
}

//------------------------------------------------------------------------------

void destroyImage(VkDevice device, const image_t& image)
{
    vkDestroyImageView(device, image.view, nullptr);
    vkDestroyImage(device, image.image, nullptr);
    vkFreeMemory(device, image.memory, nullptr);
}
