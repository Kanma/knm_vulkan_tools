#version 450

// Uniforms
layout(binding = 0) uniform uniforms_t
{
    mat4 view;
    mat4 projection;
} ubo;

// Push constants
layout(push_constant) uniform mesh_constants_t
{
    mat4 model;
} mesh;

// Inputs
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vColor;

// Outputs
layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outColor;


void main()
{
    gl_Position = ubo.projection * ubo.view * mesh.model * vec4(vPosition, 1.0);
    outTexCoord = vTexCoord;
    outColor = vColor;
}
