/*
SPDX-FileCopyrightText: 2023 Philip Abbet

SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>

SPDX-License-Identifier: MIT
*/

#pragma once

#include <knm_vulkan_tools.hpp>


struct uniforms_buffer_t
{
    // The buffer
    VkBuffer buffer;

    // The device memory allocated for the buffer
    VkDeviceMemory memory;

    void* mapped;
};



//------------------------------------------------------------------------------------
// Creates a list of buffers that will contain the uniforms (one for each
// frame-in-flight). Its content will need to be updated each frame.
//------------------------------------------------------------------------------------
void createUniformBuffers(
    const knm::vk::Application* app, VkDevice device, VkDeviceSize bufferSize,
    uint32_t nbFramesInFlight, std::vector<uniforms_buffer_t>& buffers
);


//------------------------------------------------------------------------------------
// Destroy the resources used by a list of uniforms buffers
//------------------------------------------------------------------------------------
void destroyUniformBuffers(
    VkDevice device, const std::vector<uniforms_buffer_t>& buffers
);
