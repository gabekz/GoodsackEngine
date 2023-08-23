// ---------------------- Vertex -----------------

#shader vertex
#version 420 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out VS_OUT { vec2 texCoords; }
vs_out;

uniform vec2 u_position = vec2(0, 0);
uniform vec2 u_viewport = vec2(1920, 1080);
uniform float u_scale   = 1;

void
main()
{
    gl_Position = vec4(
      u_scale * 2 * (a_Position.xy + u_position.xy) / u_viewport.xy - 1, 0, 1);
    vs_out.texCoords = a_TexCoords;
}

// ---------------------- Fragment -----------------

#shader fragment
#version 420 core

layout(binding = 0) uniform sampler2D t_Texture;

uniform bool u_using_texture = false;

uniform vec4 u_color = vec4(1);
out vec4 out_color;

in VS_OUT { vec2 texCoords; }
fs_in;

void
main()
{
    vec4 pixel = (u_using_texture)
                   ? texture(t_Texture, fs_in.texCoords) * u_color
                   : u_color;

    if (pixel.rgb == vec3(0)) discard;

    out_color = pixel * u_color;
}
