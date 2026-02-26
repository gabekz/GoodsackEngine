#shader vertex
#version 450

#define SKINNED 0

const int MAX_BONES   = 50; // max joints allowed in a skeleton
const int MAX_WEIGHTS = 4;  // max weights allowed
const int MAX_CAMERAS = 4;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

#if SKINNED
layout(location = 4) in vec4 a_Joints;
layout(location = 5) in vec4 a_Weights;
#endif

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
}
ubo;

// NOTE: every shader (stage) can only have 1 push_constant block
layout(push_constant) uniform push_constant
{
    mat4 model;
    mat4 skinned_matrices[MAX_BONES];
}
ps;

layout(location = 0) out VS_OUT { vec2 fragTexCoord; }
vs_out;

//-----------------------------------------------------------------------------
vec4
_calculate_vertex_skinning()
{
#if SKINNED
    vec4 totalLocalPos = vec4(0.0);
    for (int i = 0; i < MAX_WEIGHTS; i++)
    {
        mat4 skinnedTransform = ps.skinned_matrices[int(a_Joints[i])];
        vec4 posePos          = skinnedTransform * vec4(inPosition, 1.0);
        totalLocalPos += posePos * a_Weights[i];
    }
    return totalLocalPos;
#else
    return vec4(inPosition, 1.0);
#endif // SKINNED
}
//-----------------------------------------------------------------------------

void
main()
{
    vec4 pos = _calculate_vertex_skinning();

    gl_Position         = ubo.proj * ubo.view * ps.model * pos;
    vs_out.fragTexCoord = inTexCoord;
}

#shader fragment
#version 450

// uniform
layout(set = 0, binding = 1) uniform sampler2D texSampler;

// in
layout(location = 0) in VS_OUT { vec2 fragTexCoord; }
fs_in;

// out
layout(location = 0) out vec4 outColor;

void
main()
{
    vec4 texColor = texture(texSampler, fs_in.fragTexCoord);

    float gamma  = 2.2;
    outColor.rgb = pow(texColor.rgb, vec3(1.0 / gamma));
    // outColor.rgb = vec3(0, 1, 1);
}
