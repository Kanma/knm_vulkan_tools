/*
SPDX-FileCopyrightText: 2023 Philip Abbet

SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>

SPDX-License-Identifier: MIT
*/

#include "geometry.h"

#include <unordered_map>
#include <tiny_obj_loader.h>

using namespace knm::vk;


/********************************* INTERNAL FUNCTIONS ***********************************/

//----------------------------------------------------------------------------------------
// Load the vertices and indices of the model from an OBJ file
//----------------------------------------------------------------------------------------
void loadFile(
    const std::string& filename, std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices
)
{
    // Parse the OBJ file
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
        throw std::runtime_error(warn + err);

    // Combine all shapes in a single model
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            // Note that in OBJ files, a vertical texture coordinate of 0 means the
            // bottom of the image but for us 0 means the top
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}


//------------------------------------------------------------------------------------
// Creates a vertex buffer containing the vertices of the geometry
//------------------------------------------------------------------------------------
void createVertexBuffer(
    const knm::vk::Application* app, VkDevice device, VkCommandPool commandPool,
    const std::vector<Vertex>& vertices, geometry_t& geometry
)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // Create a staging buffer (usable on the CPU side)
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    app->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    // Copy the vertex data to the staging buffer
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Create the buffer (on the GPU)
    app->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        geometry.vertexBuffer,
        geometry.vertexBufferMemory
    );

    // Copy the data from CPU to GPU
    app->copyBuffer(commandPool, stagingBuffer, geometry.vertexBuffer, bufferSize);

    // Cleanup of the staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}


//------------------------------------------------------------------------------------
// Creates an index buffer containing the indices of the geometry
//------------------------------------------------------------------------------------
void createIndexBuffer(
    const knm::vk::Application* app, VkDevice device, VkCommandPool commandPool,
    const std::vector<uint32_t>& indices, geometry_t& geometry
)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    geometry.nbIndices = static_cast<uint32_t>(indices.size());

    // Create a staging buffer (usable on the CPU side)
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    app->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    // Copy the indices to the staging buffer
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Create the buffer (on the GPU)
    app->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        geometry.indexBuffer,
        geometry.indexBufferMemory
    );

    // Copy the data from CPU to GPU
    app->copyBuffer(commandPool, stagingBuffer, geometry.indexBuffer, bufferSize);

    // Cleanup of the staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}


/********************************** PUBLIC FUNCTIONS ************************************/

void createGeometry(
    const knm::vk::Application* app, VkDevice device, VkCommandPool commandPool,
    const std::string& filename, geometry_t& geometry
)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    loadFile(filename, vertices, indices);
    createVertexBuffer(app, device, commandPool, vertices, geometry);
    createIndexBuffer(app, device, commandPool, indices, geometry);
}

//------------------------------------------------------------------------------

void destroyGeometry(VkDevice device, const geometry_t& geometry)
{
    vkDestroyBuffer(device, geometry.indexBuffer, nullptr);
    vkFreeMemory(device, geometry.indexBufferMemory, nullptr);

    vkDestroyBuffer(device, geometry.vertexBuffer, nullptr);
    vkFreeMemory(device, geometry.vertexBufferMemory, nullptr);
}
