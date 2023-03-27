#shader vertex
#version 430 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTexCoords;

out vec2 texCoords;

void
main()
{
    gl_Position = vec4(inPos.x, inPos.y, 0.0, 1.0);
    texCoords   = inTexCoords;
}

#shader fragment
#version 430 core

out float FragColor;
in vec2 texCoords;

layout(binding = 1) uniform sampler2D t_Input;

void
main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(t_Input, 0));
    float result   = 0.0;
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(t_Input, texCoords + offset).r;
        }
    }
    FragColor = result / (4.0 * 4.0);
}