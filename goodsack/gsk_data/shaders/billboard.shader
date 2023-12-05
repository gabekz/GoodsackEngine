#shader vertex
#version 420

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Camera
{
    vec3 position;
    mat4 projection;
    mat4 view;
}
s_Camera;

uniform vec3 u_Position = vec3(0);

out VS_OUT { vec2 texCoords; }
vs_out;

void
main()
{

    vec3 cameraRightWorld = {
      s_Camera.view[0][0], s_Camera.view[1][0], s_Camera.view[2][0]};
    vec3 cameraUpWorld = {
      s_Camera.view[0][1], s_Camera.view[1][1], s_Camera.view[2][1]};

    vec3 positionWorld = u_Position + 0.05 * a_Position.x * cameraRightWorld +
                         0.05 * a_Position.y * cameraUpWorld;

    vs_out.texCoords = a_TexCoords;
    gl_Position =
      s_Camera.projection * s_Camera.view * vec4(positionWorld, 1.0);
}

#shader fragment
#version 420

in VS_OUT { vec2 texCoords; }
fs_in;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D t_Texture;

void
main()
{
    vec4 pixel = texture(t_Texture, fs_in.texCoords);

    // if(pixel.a < 0.1)
    //  discard;

    FragColor = pixel * vec4(1, 1, 1, 1);
}