#version 450

//layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

void main() {
    //outColor = texture(texSampler, fragTexCoord);
    outColor = vec4(1.0);
}
