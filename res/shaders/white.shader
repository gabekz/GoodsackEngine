#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;

layout (std140) uniform Camera {
    mat4 projection;
    mat4 view;
} s_Camera;

uniform mat4 u_Model;

void main() {
   gl_Position = s_Camera.projection * s_Camera.view * u_Model * vec4(position, 1.0);
}

#shader fragment
#version 330 core

uniform vec4 u_LightColor;

layout(location = 0) out vec4 color;

void main()
{
   color = u_LightColor;
}
