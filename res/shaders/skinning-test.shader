// ---------------------- Vertex -----------------
#shader vertex
#version 420 core

const int MAX_BONES   = 50; // max joints allowed in a skeleton
const int MAX_WEIGHTS = 4;  // max weights allowed

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec2 aNormals;

layout(location = 3) in vec4 aJoints;
layout(location = 4) in vec4 aWeights;

layout(std140, binding = 0) uniform Camera
{
    vec3 position;
    mat4 projection;
    mat4 view;
}
s_Camera;

uniform mat4 u_SkinnedMatrices[MAX_BONES];
uniform mat4 u_Model;

out vec4 vertexColor;
out vec2 texCoords;

vec4
vertex_weight_color(int bone)
{
    vec4 baseColor = vec4(0.05, 0.0, 0.0, 1.0);

    vec4 color = baseColor;
    for (int i = 0; i < MAX_WEIGHTS; i++) {
        if (int(aJoints[i]) == bone) { color.g += aWeights[i]; }
    }
    return color;
}

vec4
calculate_vertex_skinning()
{
    vec4 totalLocalPos = vec4(0.0);
    for (int i = 0; i < MAX_WEIGHTS; i++) {
        mat4 skinnedTransform = u_SkinnedMatrices[int(aJoints[i])];
        vec4 posePos          = skinnedTransform * vec4(aPosition, 1.0);
        totalLocalPos += posePos * aWeights[i];
    }
    return totalLocalPos;
}

void
main()
{
    vec4 skinnedPos = calculate_vertex_skinning();

    gl_Position = s_Camera.projection * s_Camera.view * u_Model * skinnedPos;
    vertexColor = vec4(1.0);
    texCoords   = aTexCoords;
}

// ---------------------- Fragment -----------------
#shader fragment
#version 420 core

in vec4 vertexColor;
in vec2 texCoords;

// in uvec4 outJoints;
// in vec4 outWeights;

out vec4 color;

void
main()
{
    color = vertexColor;
}