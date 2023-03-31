#version 450

// Uniforms
layout(binding = 1) uniform sampler2D texSampler;

// Inputs
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

// Outputs
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(texSampler, fragTexCoord);
}
