/*
SPDX-FileCopyrightText: 2023 Philip Abbet

SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>

SPDX-License-Identifier: MIT
*/

#pragma once

#include <knm_vulkan_tools.hpp>

#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>


//----------------------------------------------------------------------------------------
// Contains all the informations about a vertex. The vertex shader must declare a structure
// with the same fields of equivalent types.
//----------------------------------------------------------------------------------------
struct Vertex
{
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 color;
    alignas(8)  glm::vec2 texCoord;

    // Describes at which rate to load data from memory throughout the vertices
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    // Describes how to extract vertex attributes from a chunk of vertex data
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }

    // Needed to deduplicate vertices from the OBJ file
    bool operator==(const Vertex& other) const
    {
        return (pos == other.pos) && (color == other.color) && (texCoord == other.texCoord);
    }
};



// Needed to deduplicate vertices from the OBJ file
namespace std {
    template<> struct hash<Vertex>
    {
        size_t operator()(Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}



struct geometry_t
{
    // The vertex buffer and its device memory
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    // The index buffer and its device memory
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    uint32_t nbIndices;
};



void createGeometry(
    const knm::vk::Application* app, VkDevice device, VkCommandPool commandPool,
    const std::string& filename, geometry_t& geometry
);

void destroyGeometry(VkDevice device, const geometry_t& geometry);
