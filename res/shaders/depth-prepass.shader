#shader vertex
#version 420 core
layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec3 a_Normal;

const bool INVERTED_NORMALS = false;

uniform mat4 u_Model;

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

out VS_OUT
{
    vec3 fragPos;
    // vec2 texCoord;
    vec3 normal;
}
vs_out;

void
main()
{
    CameraData camera = s_Camera.cameras[u_render_layer];

    vec4 viewPos   = camera.view * u_Model * vec4(a_Position, 1.0);
    vs_out.fragPos = viewPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(camera.view * u_Model)));
    vs_out.normal = normalMatrix * (INVERTED_NORMALS ? -a_Normal : a_Normal);

    gl_Position = camera.projection * viewPos;
}

#shader fragment
#version 420 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;

in VS_OUT
{
    vec3 fragPos;
    vec3 normal;
}
fs_in;

void
main()
{
    gPosition = fs_in.fragPos;
    gNormal   = normalize(fs_in.normal);
}
