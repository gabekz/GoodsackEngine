#shader vertex
#version 420 core

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
calculate_vertex_skinning()
{
    vec4 totalLocalPos = vec4(0.0);
    for (int i = 0; i < MAX_WEIGHTS; i++) {
        mat4 skinnedTransform = u_SkinnedMatrices[int(a_Joints[i])];
        vec4 posePos          = skinnedTransform * vec4(a_Position, 1.0);
        totalLocalPos += posePos * a_Weights[i];
    }
    return totalLocalPos;
}
//-----------------------------------------------------------------------------

void
main()
{
    gl_Position = u_LightSpaceMatrix * u_Model * calculate_vertex_skinning();
}

#shader fragment
#version 420 core

void
main()
{
    //
}
