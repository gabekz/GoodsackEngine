#version 420 core
#extension GL_EXT_vulkan_glsl_relaxed : require

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;
layout(location = 2) in vec3 a_Normal;

layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout (std140, set = 0, binding = 0) uniform Camera {
    vec3 position;
    mat4 projection;
    mat4 view;
} s_Camera;

layout (std140, binding = 1) uniform Light {
    vec3 position;
    vec4 color;
} s_Light;

uniform mat4 u_Model;

out VS_OUT {
    vec3 position;
    vec2 texCoords;
    vec3 normal;
    mat3 tbn;
    vec3 camPos;
    vec3 lightPos;
    vec3 lightColor;
} vs_out;

void main() {
    gl_Position = s_Camera.projection * s_Camera.view * u_Model * vec4(a_Position, 1.0);
    vs_out.texCoords = a_TexCoords;

    // TODO: get true worldPosition
    vs_out.position = vec3(0);

    vs_out.camPos = s_Camera.position;

    vs_out.lightPos = s_Light.position;
    vs_out.lightColor = s_Light.color.xyz * 4;

    vec3 t = normalize(vec3(u_Model * vec4(a_Tangent,   0.0)));
    vec3 b = normalize(vec3(u_Model * vec4(a_Bitangent, 0.0)));
    vec3 n = normalize(vec3(u_Model * vec4(a_Normal,    0.0)));
    //t = normalize(t - dot(t, n) * n);
    //vec3 b = cross(n, t);
    vs_out.tbn= mat3(t, b, n);
    vs_out.normal = a_Normal;
}
