#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;

uniform mat4 model;
uniform mat4 camMatrix;

void main()
{
   gl_Position = camMatrix * model * vec4(position, 1.0);
}

#shader fragment
#version 330 core

uniform vec4 u_LightColor;

layout(location = 0) out vec4 color;

void main()
{
   color = u_LightColor;
}