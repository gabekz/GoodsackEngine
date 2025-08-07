// ---------------------- Vertex -----------------
#shader vertex

const int MAX_BONES   = 50; // max joints allowed in a skeleton
const int MAX_WEIGHTS = 4;  // max weights allowed
const int MAX_CAMERAS = 4;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;

layout(location = 4) in vec4 a_Joints;
layout(location = 5) in vec4 a_Weights;

uniform bool u_InvertedNormals = false;
uniform mat4 u_Model;
uniform mat4 u_SkinnedMatrices[MAX_BONES];

struct CameraData
{
    vec4 position;
    mat4 projection;
    mat4 view;
};

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

//-----------------------------------------------------------------------------
vec4
_calculate_vertex_skinning()
{
#if SKINNED
    vec4 totalLocalPos = vec4(0.0);
    for (int i = 0; i < MAX_WEIGHTS; i++)
    {
        mat4 skinnedTransform = u_SkinnedMatrices[int(a_Joints[i])];
        vec4 posePos          = skinnedTransform * vec4(a_Position, 1.0);
        totalLocalPos += posePos * a_Weights[i];
    }
    return totalLocalPos;
#else
    return vec4(a_Position, 1.0);
#endif // SKINNED
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
main()
{
    CameraData camera = s_Camera.cameras[u_render_layer];

    vec4 viewPos   = camera.view * u_Model * _calculate_vertex_skinning();
    vs_out.fragPos = viewPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(camera.view * u_Model)));
    vs_out.normal = normalMatrix * (u_InvertedNormals ? -a_Normal : a_Normal);
    vs_out.normal = normalize(vs_out.normal);
    // vs_out.normal = normalize(a_Normal);

    gl_Position      = camera.projection * viewPos;
    vs_out.texCoords = a_TexCoords;

    // TBN
    vec3 t = vec3(u_Model * vec4(a_Tangent, 0.0));
    vec3 b = vec3(u_Model * vec4(cross(a_Tangent, vs_out.normal), 0.0));
    vec3 n = vec3(u_Model * vec4(vs_out.normal, 0.0));

    vs_out.tbn = mat3(t, b, n);
}
//-----------------------------------------------------------------------------

// ---------------------- Fragment -----------------
#shader fragment

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out uvec3 gPicker;

layout(binding = 10) uniform sampler2D t_NormalMap;

uniform uint u_entity_index = 0;

in VS_OUT
{
    vec3 fragPos;
    vec2 texCoords;
    vec3 normal;
    mat3 tbn;
}
fs_in;

vec3
calcNormal(float strength)
{
    vec3 n = texture(t_NormalMap, fs_in.texCoords).rgb;
    n      = n * 2.0 - 1.0;
    n.xy *= strength;
    n = normalize(fs_in.tbn * n);
    return n;
}

void
main()
{
    gPosition = fs_in.fragPos;
    gNormal   = calcNormal(1.0f);
    gPicker   = uvec3(u_entity_index, 0, gl_PrimitiveID);
}
