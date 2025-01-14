#shader vertex
#version 420 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 texCoord;

void
main()
{
    gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0);
    texCoord    = aTexCoord;
}

#shader fragment
#version 420 core

layout(binding = 0) uniform sampler2D srcTexture;
uniform float u_filter_radius = 0.05;
uniform float u_AspectRatio   = 1.77;

in vec2 texCoord;
layout(location = 0) out vec4 upsample;

void
main()
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = u_filter_radius;
    float y = u_filter_radius * u_AspectRatio;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(srcTexture, vec2(texCoord.x - x, texCoord.y + y)).rgb;
    vec3 b = texture(srcTexture, vec2(texCoord.x, texCoord.y + y)).rgb;
    vec3 c = texture(srcTexture, vec2(texCoord.x + x, texCoord.y + y)).rgb;

    vec3 d = texture(srcTexture, vec2(texCoord.x - x, texCoord.y)).rgb;
    vec3 e = texture(srcTexture, vec2(texCoord.x, texCoord.y)).rgb;
    vec3 f = texture(srcTexture, vec2(texCoord.x + x, texCoord.y)).rgb;

    vec3 g = texture(srcTexture, vec2(texCoord.x - x, texCoord.y - y)).rgb;
    vec3 h = texture(srcTexture, vec2(texCoord.x, texCoord.y - y)).rgb;
    vec3 i = texture(srcTexture, vec2(texCoord.x + x, texCoord.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    vec3 usamp = e * 4.0;
    usamp += (b + d + f + h) * 2.0;
    usamp += (a + c + g + i);
    usamp *= 1.0 / 16.0;
    upsample = vec4(usamp, 1);
}