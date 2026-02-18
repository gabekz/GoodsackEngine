#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 fragTexCoord;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
}
ubo;

// NOTE: every shader (stage) can only have 1 push_constant block
layout(push_constant) uniform push_constant { mat4 model; }
ps;

void
main()
{
    gl_Position  = ubo.proj * ubo.view * ps.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}
