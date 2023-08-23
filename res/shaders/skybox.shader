#shader vertex
#version 420 core
layout(location = 0) in vec3 aPos;

struct CameraData
{
    vec4 position;
    mat4 projection;
    mat4 view;
};

const int MAX_CAMERAS = 4;

layout(std140, binding = 0) uniform Camera { CameraData cameras[MAX_CAMERAS]; }
s_Camera;

uniform int u_render_layer = 0; // default render layer (a.k.a. camera target
                                // that we want to render with)

out vec3 TexCoords;

mat3
mat3_emu(mat4 m4)
{
    return mat3(m4[0][0],
                m4[0][1],
                m4[0][2],
                m4[1][0],
                m4[1][1],
                m4[1][2],
                m4[2][0],
                m4[2][1],
                m4[2][2]);
}

void
main()
{

    CameraData camera = s_Camera.cameras[u_render_layer];

    TexCoords   = aPos;
    mat4 view   = mat4(mat3_emu(camera.view));
    vec4 pos    = camera.projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}

#shader fragment
#version 420 core
out vec4 FragColor;

uniform int u_draw_blur = 0;
in vec3 TexCoords;

layout(binding = 0) uniform samplerCube skybox;

void
main()
{
    vec3 envColor = (u_draw_blur > 0) ? textureLod(skybox, TexCoords, 1).rgb
                                      : texture(skybox, TexCoords).rgb;

    // envColor = envColor / (envColor + vec3(1.0));
    // envColor = pow(envColor, vec3(1.0/2.2));

    FragColor = vec4(envColor, 1.0);
}
