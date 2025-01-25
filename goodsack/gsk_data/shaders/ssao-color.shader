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

in vec2 texCoords;
out float FragColor;

// layout(location = 0) out float FragColor;

void
main()
{
    vec3 fragPos   = texture(gPosition, texCoords).xyz;
    vec3 normal    = texture(gNormal, texCoords).rgb;
    normal         = -normal;
    vec3 randomVec = texture(t_Noise, texCoords * NOISE_SCALE).xyz;
    // vec3 randomVec = normalize(vec3(0.1, 0.1, 0));

    // create TBN
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn       = mat3(tangent, bitangent, normal);

    int kernelSize = u_kernelSize;
    float radius   = u_radius;
    float bias     = u_bias;

    // iterate over the sample kernel and calc occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; i++)
    {
        // sample position (view space)
        vec3 samplePos = tbn * u_samples[i];
        samplePos      = fragPos + samplePos * radius;

        // sample position (clip space)
        vec4 offset = vec4(samplePos, 1.0);
        offset      = s_Camera.cameras[u_render_layer].projection * offset;
        offset.xyz /= offset.w;              // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform range 0.0 - 1.0

        // get (potential) occluder corresponding to sample pos in screen space
        vec3 occluderPos = texture(gPosition, offset.xy).rgb;

        float rangeCheck =
          smoothstep(0.0, 1.0, radius / length(occluderPos - fragPos));

        // in view space, greater z values are closer to camera
        occlusion +=
          (occluderPos.z >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    // FragColor = 1.0 - (occlusion / kernelSize);
    FragColor = clamp(1.0 - (occlusion / kernelSize), 0.0, 1.0);
}