#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;

out vec2 v_TexCoords;

uniform mat4 model;
uniform mat4 camMatrix;

void main()
{
   //gl_Position = vec4(position.x, position.y, position.z, 1.0);      
   gl_Position = camMatrix * model * vec4(position, 1.0);
   v_TexCoords = texCoords;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
in vec2 v_TexCoords;

uniform vec4 u_Color;
uniform sampler2D u_Texture;
uniform vec4 u_LightColor;

out vec4 FragColor;

void main()
{
   vec4 texColor = texture(u_Texture, v_TexCoords);
   //color = vec4(0.5, 0.0, 0.8, 1.0);
   FragColor = u_LightColor * texColor;
}
