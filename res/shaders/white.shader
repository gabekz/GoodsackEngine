#shader vertex
#version 420 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;

layout (std140, binding = 0) uniform Camera {
    vec3 position;
    mat4 projection;
    mat4 view;
} s_Camera;

uniform mat4 u_Model;

void main() {
   gl_Position = s_Camera.projection * s_Camera.view * u_Model * vec4(position, 1.0);
}

#shader fragment
#version 420 core

layout(location = 0) out vec4 color;

layout (std140, binding = 1) uniform Light {
    vec3 position;
    vec4 color;

} s_Light;

void main()
{
   color = s_Light.color;
}
