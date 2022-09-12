// ---------------------- Vertex -----------------
#shader vertex
#version 420 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;
layout(location = 2) in vec3 a_Normal;

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
} vs_out;

void main() {
   gl_Position = s_Camera.projection * s_Camera.view * u_Model *
       vec4(a_Position, 1.0);

    // rotated normals?
    mat3 normalMatrix = mat3(u_Model); //mvp
    normalMatrix  = inverse(normalMatrix);
    normalMatrix  = transpose(normalMatrix);
    vs_out.normal = normalize(normalMatrix * a_Normal);

   vs_out.crntPos = vec3(u_Model * vec4(a_Position, 1.0));
   vs_out.texCoords = a_TexCoords;
   vs_out.camPos = s_Camera.position;
   //v_Normal = vec3(u_Model.x, u_Model.y, u_Model.z) * normal;
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
} fs_in;

uniform sampler2D u_Texture;

out vec4 FragColor;

// Note: Directional Light is a static lightposition w/o inten
vec4 pointLight() {
    // Light attenuation
    vec3 lightVec = (s_Light.position - fs_in.crntPos);
    float dist = length(lightVec);
    float a = 5.00f;
    float b = 1.00f;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

    // Diffuse Lighting
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(fs_in.normal, lightDirection), 0.0f);

    // Ambient Light
    float ambient = 0.2f;

    // Specular Light
    float specularLight = 0.50f;
    vec3 viewDirection = normalize(fs_in.camPos - fs_in.crntPos);
    vec3 reflectionDirection = reflect(-lightDirection, fs_in.normal);
    float specularAmount =
      pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
    float specular = specularAmount * specularLight;

    return ((diffuse * inten + ambient) + (specular * inten)) * s_Light.color;
}

void main() {
    vec4 texColor = texture(u_Texture, fs_in.texCoords);
    FragColor = texColor * pointLight();
}
