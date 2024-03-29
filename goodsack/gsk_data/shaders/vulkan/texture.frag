#version 450

// in
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

// out
layout(location = 0) out vec4 outColor;

// uniform
layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
