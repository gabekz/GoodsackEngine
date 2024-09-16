#shader vertex
#version 420 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 a_Normal;

out vec2 v_TexCoords;
out vec3 v_norm;

struct CameraData
{
    vec4 position;
    mat4 projection;
    mat4 view;
};

uniform mat4 u_Model;

uniform int u_render_layer = 0; // default render layer (a.k.a. camera target
                                // that we want to render with)

const int MAX_CAMERAS = 4;

layout(std140, binding = 0) uniform Camera { CameraData cameras[MAX_CAMERAS]; }
s_Camera;

void main()
{
    CameraData camera = s_Camera.cameras[u_render_layer];
    gl_Position = camera.projection * camera.view * u_Model * vec4(position, 1);

   v_TexCoords = texCoords;
   v_norm = normalize(a_Normal);
}

#shader fragment
#version 420 core

layout(location = 0) out vec4 color;
in vec2 v_TexCoords;
in vec3 v_norm;

uniform vec4 u_Color;
uniform sampler2D u_Texture;
uniform vec4 u_LightColor;

out vec4 FragColor;

void main()
{
   vec4 texColor = texture(u_Texture, v_TexCoords);
   //color = vec4(0.5, 0.0, 0.8, 1.0);
   //FragColor = u_LightColor * texColor;
   FragColor = texColor;
}
