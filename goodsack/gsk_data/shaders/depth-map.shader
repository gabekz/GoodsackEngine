// ---------------------- Vertex -----------------
#shader vertex

const int MAX_BONES   = 50; // max joints allowed in a skeleton
const int MAX_WEIGHTS = 4;  // max weights allowed

layout(location = 0) in vec3 a_Position;

// skinning data
layout(location = 4) in vec4 a_Joints;
layout(location = 5) in vec4 a_Weights;

uniform mat4 u_LightSpaceMatrix;
uniform mat4 u_Model;
uniform mat4 u_SkinnedMatrices[MAX_BONES];

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
mat4
_calculate_vertex_skinning_m4()
{
#if SKINNED
    mat4 totalLocalPos = mat4(0.0);
    for (int i = 0; i < MAX_WEIGHTS; i++)
    {
        mat4 skinnedTransform = u_SkinnedMatrices[int(a_Joints[i])];
        totalLocalPos += skinnedTransform * a_Weights[i];
    }
    return totalLocalPos;
#endif // SKINNED
    return mat4(1.0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
main()
{
    mat4 boneTransform = _calculate_vertex_skinning_m4();
    vec4 worldPos      = u_Model * boneTransform * vec4(a_Position, 1.0);

    gl_Position = u_LightSpaceMatrix * worldPos;
}
//-----------------------------------------------------------------------------

// ---------------------- Fragment -----------------
#shader fragment

void
main()
{
    // Empty
}
