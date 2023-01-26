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

layout(binding = 0) uniform sampler2D skybox;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{    
    vec2 uv = SampleSphericalMap(normalize(localPos));
    vec3 color = texture(skybox, uv).rgb;

    FragColor = vec4(color, 1.0);
}
