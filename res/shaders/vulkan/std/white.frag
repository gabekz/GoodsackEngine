#version 450

// in
layout(location = 0) in vec2 fragTexCoord;

// out
layout(location = 0) out vec4 outColor;

// uniform
layout(binding = 1) uniform sampler2D texSampler;

void main() {
    vec4 texColor = texture(texSampler, fragTexCoord);

    float gamma = 2.2;
    outColor.rgb = pow(texColor.rgb, vec3(1.0/gamma));
}
