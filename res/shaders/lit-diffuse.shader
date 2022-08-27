
// ---------------------- Vertex -----------------
#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 normal;

uniform mat4 u_NormMat;
uniform mat4 u_Model;
uniform mat4 u_CamMatrix;

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_CrntPos;

void main()
{
   //gl_Position = vec4(position.x, position.y, position.z, 1.0);      
   gl_Position = u_CamMatrix * u_Model * vec4(position, 1.0);
   v_TexCoords = texCoords;

    // rotated normals?
    mat3 normalMatrix = mat3(u_Model); //mvp
    normalMatrix = inverse(normalMatrix);
    normalMatrix = transpose(normalMatrix);
    v_Normal = normalize(normalMatrix * normal);
    //v_Normal = normalize(normal);

   v_CrntPos = vec3(u_Model * vec4(position, 1.0));
   //v_Normal = vec3(u_Model.x, u_Model.y, u_Model.z) * normal;
}

// ---------------------- Fragment -----------------

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
in vec2 v_TexCoords;
in vec3 v_Normal;
in vec3 v_CrntPos;

uniform sampler2D u_Texture;
uniform vec4 u_LightColor;
uniform vec3 u_LightPosition;
uniform vec3 u_CamPos;

out vec4 FragColor;

// Note: Directional Light is a static lightposition w/o inten
vec4 pointLight() {
    // Light attenuation
    vec3 lightVec = (u_LightPosition - v_CrntPos);
    float dist = length(lightVec);
    float a = 5.00f;
    float b = 1.00f;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

    // Diffuse Lighting
    vec3 lightDirection = normalize(lightVec);
    float diffuse = max(dot(v_Normal, lightDirection), 0.0f);

    // Ambient Light
    float ambient = 0.2f;

    // Specular Light
    float specularLight = 0.50f;
    vec3 viewDirection = normalize(u_CamPos - v_CrntPos);
    vec3 reflectionDirection = reflect(-lightDirection, v_Normal);
    float specularAmount =
      pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
    float specular = specularAmount * specularLight;

    return ((diffuse * inten + ambient) + (specular * inten)) * u_LightColor;
}

void main() {

    vec4 texColor = texture(u_Texture, v_TexCoords);
    FragColor = texColor * pointLight();
}
