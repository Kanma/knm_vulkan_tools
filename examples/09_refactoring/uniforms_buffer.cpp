/*
SPDX-FileCopyrightText: 2023 Philip Abbet

SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>

SPDX-License-Identifier: MIT
*/

#include "uniforms_buffer.h"

using namespace knm::vk;


void createUniformBuffers(
    const knm::vk::Application* app, VkDevice device, VkDeviceSize bufferSize,
    uint32_t nbFramesInFlight, std::vector<uniforms_buffer_t>& buffers
)
{
    buffers.resize(nbFramesInFlight);

    for (size_t i = 0; i < nbFramesInFlight; ++i)
    {
        app->createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            buffers[i].buffer,
            buffers[i].memory
        );

        vkMapMemory(device, buffers[i].memory, 0, bufferSize, 0, &buffers[i].mapped);
    }
}

//------------------------------------------------------------------------------

void destroyUniformBuffers(
    VkDevice device, const std::vector<uniforms_buffer_t>& buffers
)
{
    for (size_t i = 0; i < buffers.size(); ++i)
    {
        vkDestroyBuffer(device, buffers[i].buffer, nullptr);
        vkFreeMemory(device, buffers[i].memory, nullptr);
    }
}
