// ---------------------- Vertex -----------------
#shader vertex
#version 420 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;

layout (std140, binding = 0) uniform Camera {
    vec3 position;
    mat4 projection;
    mat4 view;
} s_Camera;

layout (std140, binding = 1) uniform Light {
    vec3 position;
    vec4 color;
} s_Light;

uniform mat4 u_Model;

out vec4 lightColor;

void main() {
   gl_Position = s_Camera.projection * s_Camera.view * u_Model * vec4(position, 1.0);
   lightColor = s_Light.color;
}

// ---------------------- Fragment -----------------
#shader fragment
#version 420 core

in vec4 lightColor;
out vec4 color;

void main()
{
   color = lightColor;
}
