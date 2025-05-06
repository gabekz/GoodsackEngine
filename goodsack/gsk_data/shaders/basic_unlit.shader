// ---------------------- Vertex -----------------
#shader vertex
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Camera
{
    vec3 position;
    mat4 projection;
    mat4 view;
}
s_Camera;

uniform mat4 u_Model;

out VS_OUT { vec2 texCoords; }
vs_out;

void
main()
{
    gl_Position =
      s_Camera.projection * s_Camera.view * u_Model * vec4(a_Position, 1.0);

    vs_out.texCoords = a_TexCoords;
}

// ---------------------- Fragment -----------------
#shader fragment

in VS_OUT { vec2 texCoords; }
fs_in;

layout(binding = 0) uniform sampler2D t_Diffuse;

uniform vec4 u_Color = vec4(1);

out vec4 out_color;

void
main()
{
    vec4 col  = texture(t_Diffuse, fs_in.texCoords);
    out_color = col;
}