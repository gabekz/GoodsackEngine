// ---------------------- Vertex -----------------
#shader vertex
#version 420 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;
layout(location = 2) in vec3 a_Normal;

layout(location = 3) in vec3 a_Tangent;
// layout(location = 4) in vec3 a_Bitangent;

// Set 0

layout(std140, binding = 0) uniform Camera
{
    vec3 position;
    mat4 projection;
    mat4 view;
}
s_Camera;

layout(std140, binding = 1) uniform Light
{
    vec3 position;
    vec4 color;
}
s_Light;

// Set 1

uniform mat4 u_Model;
uniform mat4 u_LightSpaceMatrix;
// uniform bool u_tbn_from_shader = false;

out VS_OUT
{
    vec3 position;
    vec2 texCoords;
    vec3 normal;

    mat3 tbn;

    vec3 camPos;
    vec3 lightPos;
    vec3 lightColor;

    vec4 lightWorldSpace;
    vec4 modelWorldSpace;
}
vs_out;

void
main()
{
    vs_out.texCoords = a_TexCoords;

    vs_out.position = vec3(u_Model * vec4(a_Position, 1.0));
    vs_out.normal   = a_Normal;

    vec3 position3 = vec3(u_Model * vec4(a_Position, 1.0));

    // Model world-space (screen-space projection)
    vec4 position = s_Camera.projection * s_Camera.view * vec4(position3, 1);
    vs_out.modelWorldSpace = position;

    vs_out.camPos = s_Camera.position;

    vs_out.lightPos   = s_Light.position;
    vs_out.lightColor = s_Light.color.xyz * 4;

    // TBN
    vec3 t = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
    vec3 b = normalize(vec3(u_Model * vec4(cross(a_Tangent, a_Normal), 0.0)));
    vec3 n = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));
    vs_out.tbn = mat3(t, b, n);

    // Shadowmap
    vs_out.lightWorldSpace = u_LightSpaceMatrix * vec4(vs_out.position, 1.0f);

    gl_Position = s_Camera.projection * s_Camera.view * vec4(position3, 1);
}

// ---------------------- Fragment -----------------
#shader fragment
#version 420 core

layout(binding = 0) uniform sampler2D t_Albedo;
layout(binding = 1) uniform sampler2D t_Normal;
layout(binding = 2) uniform sampler2D t_Metallic;
layout(binding = 3) uniform sampler2D t_Specular;
layout(binding = 4) uniform sampler2D t_Ao;

layout(binding = 5) uniform samplerCube t_IrradianceMap;
layout(binding = 6) uniform samplerCube t_PrefilterMap;
layout(binding = 7) uniform sampler2D t_brdfLUT;

layout(binding = 8) uniform sampler2D t_shadowMap;
layout(binding = 9) uniform sampler2D t_ssao;

uniform int u_pcfSamples      = 6;
uniform float u_normalBiasMin = 0.0025f;
uniform float u_normalBiasMax = 0.0005f;

/*
layout (std140, set = 2, binding = 0) uniform ObjectTextures {
    sampler2D t_Albedo;
    sampler2D t_Normal;
    sampler2D t_Metallic;
    sampler2D t_Specular;
    sampler2D t_Ao;
} s_textures;

layout(std140, set = 2, binding = 1) uniform ShadowTex {
    sampler2D t_ShadowMap;
};
*/

in VS_OUT
{
    vec3 position;
    vec2 texCoords;
    vec3 normal;

    mat3 tbn;

    vec3 camPos;
    vec3 lightPos;
    vec3 lightColor;

    vec4 lightWorldSpace;
    vec4 modelWorldSpace;
}
fs_in;

const float PI      = 3.14159265359;
const float FEATHER = 0.0000001;

const float ALPHA_CUTOFF = 0.5;

// material properties
/*
vec3 albedo = vec3(1.0, 0.0, 0.0);
float metallic = 0.2;
float roughness = 0.1;
float ao = 1;
*/
vec3 albedo     = texture(t_Albedo, fs_in.texCoords).rgb;
float metallic  = texture(t_Metallic, fs_in.texCoords).r;
float roughness = texture(t_Specular, fs_in.texCoords).r;
float ao        = texture(t_Ao, fs_in.texCoords).r;

// layout(location = 0) out vec4 color;
out vec4 FragColor;

// TODO: Move to separate fg shader
//--------------
float
calcAo(float strength)
{
    // per-fragment projection
    vec3 projCoords = fs_in.modelWorldSpace.xyz / fs_in.modelWorldSpace.w;
    projCoords      = projCoords * 0.5 + 0.5;

    return pow(texture(t_ssao, projCoords.xy).r, strength);
}

// TODO: Move this to a separate fg shader
//----------------
vec3
calcNormal(float strength)
{
    vec3 n = texture(t_Normal, fs_in.texCoords).rgb;
    n      = n * 2.0 - 1.0;
    n.xy *= strength;
    n = normalize(fs_in.tbn * n);
    return n;
}

// TODO: Move to separate file to avoid redundance
float
calcShadow(vec4 lightWorldSpace, vec3 lightDir, bool pcf)
{

    vec3 projCoords = lightWorldSpace.xyz / lightWorldSpace.w;
    projCoords      = projCoords * 0.5 + 0.5;

    float biasMin  = u_normalBiasMin;
    float biasMax  = u_normalBiasMax;
    int pcfSamples = u_pcfSamples;

    // PCF disable
    if (pcfSamples <= 0) { pcf = false; }

    // oversampling correction
    if (projCoords.z > 1.0) { return 0.0; }

    float closestDepth = texture(t_shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias =
      max(biasMin * (1.0 - dot(normalize(fs_in.normal), lightDir)), biasMax);
    float shadow = 0;

    if (pcf) {
        vec2 texelSize = 1.0 / textureSize(t_shadowMap, 0);
        for (int x = -pcfSamples; x <= pcfSamples; ++x) {
            for (int y = -pcfSamples; y <= pcfSamples; ++y) {
                float pcfDepth =
                  texture(t_shadowMap, projCoords.xy + vec2(x, y) * texelSize)
                    .r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= pow((pcfSamples * 2 + 1), 2);
        return shadow;

    } else {
        shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    }

    return shadow;
}

//------------------
float
distributionGGX(float NdotH, float r)
{
    float a     = r * r;
    float a2    = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    denom       = PI * denom * denom;
    return a2 / max(denom, FEATHER); // prevent d.b.z.
}
//------------------
float
geometrySmith(float NdotV, float NdotL, float r)
{
    float a    = r + 1.0;
    float k    = (a * a) / 8.0;
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k); // Schlick GGX
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k); // Schlick GGX
    return ggx1 * ggx2;
}
//------------------
vec3
fresnelSchlick(float HdotV, vec3 bR)
{
    // baseReflectivity in range 0 to 1
    // returns range of baseReflectivity to 1
    // increases as HdotV decreases
    // (more reflective when surface viewed at larger angles)
    return bR + (1.0 - bR) * pow(1.0 - HdotV, 5.0);
}
//------------------
vec3
fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) *
                  pow(max(1.0 - cosTheta, 0.0), 5.0);
}

void
main()
{
    // alpha-cutoff
    if (texture(t_Albedo, fs_in.texCoords).a <= ALPHA_CUTOFF) discard;

    vec3 N = calcNormal(1.0);
    vec3 V = normalize(fs_in.camPos - fs_in.position);
    vec3 R = reflect(-V, N);

    float NdotV = max(dot(N, V), FEATHER);

    // calculate reflectance at normal incidence; if dia-electric
    // (like plastic) use baseReflectivity of 0.0f and it's a metal,
    // use the albedo color as baseReflectivity (metallic workflow)
    vec3 bR = vec3(0.04); // F0
    bR      = mix(bR, albedo, metallic);

    vec3 Lo  = vec3(0.0); // reflectance equation
    float sV = 0;         // shadow-coverage value
    // start of lighting equation
    const int N_LIGHTS   = 1;
    const int LIGHT_TYPE = 0; // 0 = directional
    for (int i = 0; i < N_LIGHTS; i++) {
        // Calcuate per-light radiance
        vec3 L            = normalize(fs_in.lightPos - fs_in.position);
        vec3 H            = normalize(V + L);
        float distance    = length(fs_in.lightPos - fs_in.position);
        float attenuation = 1.0 / (distance * distance); // distance^2 for Gamma
        vec3 radiance     = fs_in.lightColor * attenuation;

        // TODO: Quick hack for directional lighting
        if (LIGHT_TYPE == 0) {
            L        = normalize(fs_in.lightPos);
            radiance = fs_in.lightColor;
        }

        float NdotL = max(dot(N, L), FEATHER);
        float HdotV = max(dot(H, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);

        // Cook-Torrance BRDF
        float D = distributionGGX(
          NdotH, roughness); // larger the more micro-facets aligned to H
        float G =
          geometrySmith(NdotV, NdotL, roughness); // smaller more micro-facets
        vec3 F = fresnelSchlick(HdotV, bR);       // proportion of spec

        /*
        vec3 specular = D * G * F;
        specular /= 4.0 * NdotV * NdotL;
        */
        vec3 nominator    = D * G * F;
        float denominator = 4 * NdotV * NdotL + 0.001;
        vec3 specular     = nominator / denominator;

        // For energy conservation, the diffuse and specular light
        // can't be above 1.0 (unless the surface emits light);
        // to preserve this relationship, the diffuse component (kD)
        // should equal 1.0 - ks.
        vec3 kD = vec3(1.0) - F; // F equals ks

        // multiply kD by the inverse metalness such that only
        // non-metals have diffuse lighting, or a linear blend
        // if partly metal (pure metal have no diffuse light).
        kD *= 1.0 - metallic;

        // shadow-coverage factor
        sV = calcShadow(fs_in.lightWorldSpace, L, true);
        // sV = calcShadow(fs_in.lightWorldSpace, normalized(fs_in.lightPos),
        // true);

        // 1) angle of light to surface affect specular, not just diffuse
        // 2) we mix albedo with diffuse, but not specular
        // 3) combine visibility (shadow) factor
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    // end of lighting equation

    // ambient lighting from IBL
    vec3 F       = fresnelSchlickRoughness(max(dot(N, V), 0.0), bR, roughness);
    vec3 kD      = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = texture(t_IrradianceMap, N).rgb * albedo * kD;
    // vec3 ambient = vec3(0, 0.0001, 0.0003) * albedo * ao;

    // sample pre-filter map
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor =
      textureLod(t_PrefilterMap, reflect(-V, N), roughness * MAX_REFLECTION_LOD)
        .rgb;
    vec2 brdf     = texture(t_brdfLUT, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.r + brdf.g);

    float aoStrength = 2.5;
    vec3 ambient     = (diffuse + specular) * calcAo(aoStrength);

    // vec3 shadowColor = vec3(0.0);
    vec3 shadowColor      = fs_in.lightColor * 0.002;
    float shadowIntensity = 1.0;
    shadowColor           = (1 - (sV * (vec3(shadowIntensity) - shadowColor)));

    vec3 color = (ambient + Lo) * shadowColor;

    // HDR
    // color = color / (color + vec3(1.0));
    // Gamma
    // color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
