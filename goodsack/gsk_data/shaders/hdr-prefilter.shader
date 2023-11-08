#shader vertex
#version 420 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 localPos;

void main() {
    localPos = aPos;
    gl_Position = projection * view * vec4(localPos, 1);
}

#shader fragment
#version 420 core
out vec4 FragColor;
in vec3 localPos;

uniform float u_Roughness;

layout(binding = 0) uniform samplerCube environmentMap;

const float PI = 3.14159265359;

float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec3 importanceSampleGGX(uint i, uint sampleCount, vec3 N, float roughness) {
    vec2 hammersley = vec2(float(i)/float(sampleCount), radicalInverse_VdC(i));
    float a = roughness * roughness;

    float phi = 2.0 * PI * hammersley.x;
    float cosTheta = sqrt((1.0 - hammersley.y) / (1.0 + (a*a - 1.0) *hammersley.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

void main()
{    
    vec3 N = normalize(localPos);
    vec3 V = N;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);

    for(uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec3 H = importanceSampleGGX(i, SAMPLE_COUNT, N, u_Roughness);
        vec3 L = reflect(-V, H);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            prefilteredColor += texture(environmentMap, L).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;
    FragColor = vec4(prefilteredColor, 1.0);
}
