#shader vertex
#version 420 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;

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
    vec2 texCoords;
    vec3 normal;
    mat3 tbn;
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

    gl_Position      = camera.projection * viewPos;
    vs_out.texCoords = a_TexCoords;

    // TBN
    vec3 t = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
    vec3 b =
      normalize(vec3(u_Model * vec4(cross(a_Tangent, vs_out.normal), 0.0)));
    vec3 n = normalize(vec3(u_Model * vec4(vs_out.normal, 0.0)));

    vs_out.tbn = mat3(t, b, n);
}

#shader fragment
#version 420 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;

layout(binding = 10) uniform sampler2D t_NormalMap;

in VS_OUT
{
    vec3 fragPos;
    vec2 texCoords;
    vec3 normal;
    mat3 tbn;
}
fs_in;

void
main()
{
    gPosition = fs_in.fragPos;
    // gNormal   = normalize(fs_in.normal);
    vec3 nrml =
      normalize(texture(t_NormalMap, fs_in.texCoords).rgb * 2.0 - 1.0);
    gNormal = normalize(fs_in.tbn * nrml);
}
