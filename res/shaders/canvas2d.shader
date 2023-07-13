// ---------------------- Vertex -----------------

#shader vertex
#version 420 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out VS_OUT { vec2 texCoords; }
vs_out;

uniform float u_scale = 1;

/*
uniform mat4 projection =
[[1/1280, 0, 0, 0], [0, -(2/720), 0, 0],
[0, 0, 1, 0], [-1, 1, 0, 1]];
*/

void
main()
{
    vec2 viewport = vec2(1920, 1080);
    gl_Position   = vec4(u_scale * 2 * a_Position.xy / viewport.xy - 1, 0, 1);
    vs_out.texCoords = a_TexCoords;
}

// ---------------------- Fragment -----------------

#shader fragment
#version 420 core

layout(binding = 0) uniform sampler2D t_Texture;

out vec4 color;

in VS_OUT { vec2 texCoords; }
fs_in;

void
main()
{
    vec4 pixel = texture(t_Texture, fs_in.texCoords);
    color      = pixel * vec4(0, 1, 0, 1);
}
