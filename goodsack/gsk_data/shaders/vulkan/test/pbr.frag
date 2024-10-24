#version 420 core

layout(binding = 0) uniform sampler2D t_Albedo;
layout(binding = 1) uniform sampler2D t_Normal;
layout(binding = 2) uniform sampler2D t_Metallic;
layout(binding = 3) uniform sampler2D t_Specular;
layout(binding = 4) uniform sampler2D t_Ao;

in VS_OUT {
    vec3 position;
    vec2 texCoords;
    vec3 normal;
    mat3 tbn;
    vec3 camPos;
    vec3 lightPos;
    vec3 lightColor;
} fs_in;

const float PI = 3.14159265359;
const float FEATHER = 0.0000001;

// material properties
/*
vec3 albedo = vec3(1.0, 0.0, 0.0);
float metallic = 0.2;
float roughness = 0.1;
float ao = 1;
*/
vec3 albedo = texture(t_Albedo, fs_in.texCoords).rgb;
float metallic = texture(t_Metallic, fs_in.texCoords).r;
float roughness = texture(t_Specular, fs_in.texCoords).r;
float ao = texture(t_Ao, fs_in.texCoords).r;

//layout(location = 0) out vec4 color;
out vec4 FragColor;

//TODO: Move this to a separate fg shader
//----------------
vec3 calcNormal(float strength){
    vec3 n = texture(t_Normal, fs_in.texCoords).rgb;
    n = n * 2.0 - 1.0;
    n.xy *= strength;
    n = normalize(fs_in.tbn * n);
    return n;
}

//------------------
float distributionGGX(float NdotH, float r) {
    float a = r * r;
    float a2 = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / max(denom, FEATHER); // prevent d.b.z.
}
//------------------
float geometrySmith(float NdotV, float NdotL, float r) {
    float a = r + 1.0;
    float k = (a * a) / 8.0;
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k); // Schlick GGX
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k); // Schlick GGX
    return ggx1 * ggx2;
}
//------------------
vec3 fresnelSchlick(float HdotV, vec3 bR) {
    // baseReflectivity in range 0 to 1
    // returns range of baseReflectivity to 1
    // increases as HdotV decreases
    // (more reflective when surface viewed at larger angles)
    return bR + (1.0 - bR) * pow(1.0 - HdotV, 5.0);
}

void main() {

    //vec3 N = normalize(fs_in.normal);
    vec3 N = calcNormal(1.0);
    vec3 V = normalize(fs_in.camPos - fs_in.position);

    // calculate reflectance at normal incidence; if dia-electric
    // (like plastic) use baseReflectivity of 0.0f and it's a metal,
    // use the albedo color as baseReflectivity (metallic workflow)
    vec3 bR = mix(vec3(0.04), albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0); 

// start of lighting equation
    // Calcuate per-light radiance
    vec3 L = normalize(fs_in.lightPos - fs_in.position);
    vec3 H = normalize(V + L);
    float distance = length(fs_in.lightPos - fs_in.position);
    float attenuation = 1.0 / (distance * distance); // distance^2 for Gamma
    vec3 radiance = fs_in.lightColor * attenuation;

    // Cook-Torrance BRDF
    float NdotV = max(dot(N, V), FEATHER);
    float NdotL = max(dot(N, L), FEATHER);
    float HdotV = max(dot(H, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    float D = distributionGGX(NdotH, roughness); // larger the more micro-facets aligned to H
    float G = geometrySmith(NdotV, NdotL, roughness); // smaller more micro-facets
    vec3 F = fresnelSchlick(HdotV, bR); // proportion of spec 
    
    vec3 specular = D * G * F;
    specular /= 4.0 * NdotV * NdotL;

    // For energy conservation, the diffuse and specular light
    // can't be above 1.0 (unless the surface emits light);
    // to preserve this relationship, the diffuse component (kD)
    // should equal 1.0 - ks.
    vec3 kD = vec3(1.0) - F; // F equals ks
    
    // multiply kD by the inverse metalness such that only
    // non-metals have diffuse lighting, or a linear blend
    // if partly metal (pure metal have no diffuse light).
    kD *= 1.0 - metallic;

    // 1) angle of light to surface affect specular, not just diffuse
    // 2) we mix albedo with diffuse, but not specular
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
// end of lighting equation

    // ambient lighting
    vec3 ambient = vec3(0, 0.0001, 0.0003) * albedo * ao;
    vec3 color = ambient + Lo;
    // HDR
    color = color / (color + vec3(0.05));
    // Gamma
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
