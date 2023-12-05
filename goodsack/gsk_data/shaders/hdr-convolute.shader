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

layout(binding = 0) uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{    
    vec3 N = normalize(localPos);
    vec3 irradiance = vec3(0.0);

    vec3 right = cross(vec3(0.0, 1.0, 0.0), N);
    vec3 up = cross(N, right);

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {

            vec3 tangentSample = vec3(sin(theta) * cos(phi),
                    sin(theta) * sin(phi), cos(theta));

            vec3 sampleVec = 
                tangentSample.x * right
                + tangentSample.y * up
                + tangentSample.z * N;

            irradiance += texture(environmentMap, sampleVec).rgb
                * cos(theta) * sin(theta);
            nrSamples++;
        
        }
    }

    irradiance *= PI / nrSamples;
    FragColor = vec4(irradiance, 1.0);
}
