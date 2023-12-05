// ---------------------- Vertex -----------------
#shader vertex
#version 420 core
layout(location = 0) in vec3 position;

layout (std140, binding = 0) uniform Camera {
    vec3 position;
    mat4 projection;
    mat4 view;
} s_Camera;

uniform mat4 u_Model;

void main() {
   gl_Position = s_Camera.projection * s_Camera.view * u_Model * vec4(position, 1.0);
}

// ---------------------- Fragment -----------------
#shader fragment
#version 420 core

out vec4 color;

void main()
{
   color = vec4(1, 1, 1, 1);
}