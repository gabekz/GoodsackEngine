#shader vertex
#version 420 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;

//#define USE_NORMAL_MAP 1

const bool INVERTED_NORMALS = false;

uniform mat4 u_Model;

layout(std140, binding = 0) uniform Camera
{
    vec3 position;
    mat4 projection;
    mat4 view;
}
s_Camera;

out VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
    mat3 tbn;
}
vs_out;

void
main()
{
    vec4 viewPos   = s_Camera.view * u_Model * vec4(a_Position, 1.0);
    vs_out.fragPos = viewPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(s_Camera.view * u_Model)));
    vs_out.normal = normalMatrix * (INVERTED_NORMALS ? -a_Normal : a_Normal);

    gl_Position      = s_Camera.projection * viewPos;
    vs_out.texCoords = a_TexCoords;

    vec3 t = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
    vec3 b =
      normalize(vec3(u_Model * vec4(cross(a_Tangent, vs_out.normal), 0.0)));
    vec3 n = normalize(vec3(u_Model * vec4(vs_out.normal, 0.0)));
    // t = normalize(t - dot(t, n) * n);
    // vec3 b = cross(n, t);
    vs_out.tbn = mat3(t, b, n);
}

#shader fragment
#version 420 core

#define USING_NORMAL_MAP 1

#if USING_NORMAL_MAP
#endif

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;

layout(binding = 10) uniform sampler2D t_NormalMap;

in VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
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
