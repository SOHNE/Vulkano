#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

void main() {
    // Transform vertex to clip space
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
    
    // Pass through texture coordinates
    fragTexCoord = inTexCoord;
}
