// ---------------------- Vertex -----------------
#shader vertex
#version 420 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;
layout(location = 2) in vec3 a_Normal;

layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout (std140, binding = 0) uniform Camera {
    vec3 position;
    mat4 projection;
    mat4 view;
} s_Camera;

uniform mat4 u_Model;

out VS_OUT {
    vec2 texCoords;
    vec3 normal;
    vec3 crntPos;
    vec3 camPos;
    mat3 tbn;
    vec4 fragLightSpace;
} vs_out;

uniform mat4 u_LightSpaceMatrix;

void main() {
   gl_Position = s_Camera.projection * s_Camera.view * u_Model *
       vec4(a_Position, 1.0);

    vec3 t = normalize(vec3(u_Model * vec4(a_Tangent,   0.0)));
    vec3 b = normalize(vec3(u_Model * vec4(a_Bitangent, 0.0)));
    vec3 n = normalize(vec3(u_Model * vec4(a_Normal,    0.0)));
    //t = normalize(t - dot(t, n) * n);
    //vec3 b = cross(n, t);
    vs_out.tbn= mat3(t, b, n);

    // transposing the inverse of the normals
    mat3 normalMatrix = mat3(u_Model);
    normalMatrix  = inverse(normalMatrix);
    normalMatrix  = transpose(normalMatrix);
    vs_out.normal = normalize(normalMatrix * a_Normal);

   vs_out.crntPos = vec3(u_Model * vec4(a_Position, 1.0));
   vs_out.texCoords = a_TexCoords;
   vs_out.camPos = s_Camera.position;

   vs_out.fragLightSpace = u_LightSpaceMatrix * vec4(vs_out.crntPos, 1.0f);
}

// ---------------------- Fragment -----------------

#shader fragment
#version 420 core

layout(location = 0) out vec4 color;

layout(std140, binding = 1) uniform Light {
    vec3 position;
    vec4 color;

} s_Light;

in VS_OUT {
    vec2 texCoords;
    vec3 normal;
    vec3 crntPos;
    vec3 camPos;
    mat3 tbn;
    vec4 fragLightSpace;
} fs_in;


layout(binding = 6) uniform sampler2D shadowMap;

// default bindings (requires 4.2+)
layout(binding = 0) uniform sampler2D t_Diffuse;
layout(binding = 1) uniform sampler2D t_Normal;
layout(binding = 2) uniform sampler2D t_Specular;

out vec4 FragColor;

vec3 calcNormal(float strength){
    vec3 n = texture(t_Normal ,fs_in.texCoords).rgb;
    n = n * 2.0 - 1.0;
    n.xy *= strength;
    n = normalize(fs_in.tbn * n);
    return n;
}

float calcShadow(vec4 fragLightSpace, vec3 lightDir, bool pcf) {

    vec3 projCoords = fragLightSpace.xyz / fragLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.05 * (1.0 - dot(fs_in.normal, lightDir)), 0.005);
    float shadow = 0;
    if(pcf) {
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y<= 1; ++y) {
                float pcfDepth = 
                    texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
        return shadow;

    } else {
        shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    }

    return shadow;
}

// Note: Directional Light is a static lightposition w/o inten
vec4 light(int type) {
    // Light attenuation
    vec3 lightVec = vec3(1.0f, 1.0f, 1.0f); // default (for directional)
    float inten = 0.9f;
    if(type > 0) {
        lightVec = (s_Light.position - fs_in.crntPos);

        float dist = length(lightVec);
        float a = 2.00f;
        float b = 1.00f;
        inten = 1.0f / (a * dist * dist + b * dist + 1.0f);
    }

    // Diffuse Lighting
    //vec3 lightDirection = normalize(lightVec);
    vec3 lightDirection = normalize(max(-lightVec * -fs_in.normal, 0.0f));
    float diffuse = max(dot(fs_in.normal * calcNormal(1.0), lightDirection), 0.0f);

    // Ambient Light
    float ambient = 0.1f;

    // Specular Light
    float specular = 0.0f;
    if(diffuse != 0.0f) {
        float specularLight = 1.00f * texture(t_Specular, fs_in.texCoords).r;
        vec3 viewDirection = normalize(fs_in.camPos - fs_in.crntPos);
        vec3 reflectionDirection = reflect(-lightDirection, fs_in.normal);

        vec3 halfway = normalize(viewDirection + lightDirection);

        float specularAmount =
          //pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
          pow(max(dot(fs_in.normal, halfway), 0.0f), 8);
        specular = specularAmount * specularLight;
    }

    float shadow = calcShadow(fs_in.fragLightSpace, lightDirection, true);
    //return ((diffuse * inten + (ambient + (1.0f - shadow))) + (specular * inten)) * s_Light.color;
    return (ambient + (1.0 - shadow) * diffuse * inten + (specular * inten)) * s_Light.color;
}

void main() {
    vec4 texColor = texture(t_Diffuse, fs_in.texCoords);
    FragColor = texColor * light(0);
}
