#shader vertex
#version 420 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 texCoords;

void
main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    texCoords   = aTexCoords;
}

#shader fragment
#version 420 core

// params
uniform int u_kernelSize = 64;
uniform float u_radius   = 0.5;
uniform float u_bias     = 0.025;

uniform vec3 u_samples[64];

layout(binding = 1) uniform sampler2D gPosition;
layout(binding = 2) uniform sampler2D gNormal;
layout(binding = 3) uniform sampler2D t_Noise;

const vec2 NOISE_SCALE = vec2(1280.0 / 4, 720.0 / 4);

layout(std140, binding = 0) uniform Camera
{
    vec3 position;
    mat4 projection;
    mat4 view;
}
s_Camera;

in vec2 texCoords;
out float FragColor;

// layout(location = 0) out float FragColor;

void
main()
{
    vec3 fragPos = texture(gPosition, texCoords).xyz;
    vec3 normal  = texture(gNormal, texCoords).xyz;
    // vec3 randomVec = normalize(texture(t_Noise, texCoords *
    // NOISE_SCALE).xyz);
    vec3 randomVec = normalize(vec3(0.1, 0.1, 0));

    // create TBN
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn       = mat3(tangent, bitangent, normal);

    // iterate over the sample kernel and calc occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < u_kernelSize; i++) {
        // sample position
        vec3 smpl = tbn * u_samples[i];
        smpl      = fragPos + smpl * u_radius;

        // sample position in screen space
        vec4 offset = vec4(smpl, 1.0);
        offset      = s_Camera.projection * offset;
        offset.xyz /= offset.w;              // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform range 0.0 - 1.0

        // get (potential) occluder corresponding to sample pos in screen space
        vec3 occluderPos = texture(gPosition, offset.xy).rgb;

        float rangeCheck =
          smoothstep(0.0, 1.0, u_radius / length(fragPos - occluderPos));

        // in view space, greater z values are closer to camera
        occlusion +=
          (occluderPos.z >= smpl.z + u_bias ? 1.0 : 0.0) * rangeCheck;
    }
    FragColor = 1.0 - (occlusion / u_kernelSize);
}