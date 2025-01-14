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

uniform vec2 u_src_resolution;
uniform float u_Threshold  = 0;
uniform bool u_DoPrefilter = false;

in vec2 texCoord;
layout(location = 0) out vec4 downsample;

vec3
prefilter_threshold(vec3 c)
{
    float brightness   = max(c.r, max(c.g, c.b));
    float contribution = max(0, brightness - u_Threshold);
    contribution /= max(brightness, 0.00001);
    return c * contribution;
}

void
main()
{
    vec2 src_texel_size = 1.0 / u_src_resolution;
    float x             = src_texel_size.x;
    float y             = src_texel_size.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a =
      texture(srcTexture, vec2(texCoord.x - 2 * x, texCoord.y + 2 * y)).rgb;
    vec3 b = texture(srcTexture, vec2(texCoord.x, texCoord.y + 2 * y)).rgb;
    vec3 c =
      texture(srcTexture, vec2(texCoord.x + 2 * x, texCoord.y + 2 * y)).rgb;

    vec3 d = texture(srcTexture, vec2(texCoord.x - 2 * x, texCoord.y)).rgb;
    vec3 e = texture(srcTexture, vec2(texCoord.x, texCoord.y)).rgb;
    vec3 f = texture(srcTexture, vec2(texCoord.x + 2 * x, texCoord.y)).rgb;

    vec3 g =
      texture(srcTexture, vec2(texCoord.x - 2 * x, texCoord.y - 2 * y)).rgb;
    vec3 h = texture(srcTexture, vec2(texCoord.x, texCoord.y - 2 * y)).rgb;
    vec3 i =
      texture(srcTexture, vec2(texCoord.x + 2 * x, texCoord.y - 2 * y)).rgb;

    vec3 j = texture(srcTexture, vec2(texCoord.x - x, texCoord.y + y)).rgb;
    vec3 k = texture(srcTexture, vec2(texCoord.x + x, texCoord.y + y)).rgb;
    vec3 l = texture(srcTexture, vec2(texCoord.x - x, texCoord.y - y)).rgb;
    vec3 m = texture(srcTexture, vec2(texCoord.x + x, texCoord.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them
    // overlap, so to have an energy preserving downsample we need to make some
    // adjustments. The weights are the distributed, so that the sum of j,k,l,m
    // (e.g.) contribute 0.5 to the final color output. The code below is
    // written to effectively yield this sum. We get: 0.125*5 + 0.03125*4 +
    // 0.0625*4 = 1
    vec3 dsamp = e * 0.125;
    dsamp += (a + c + g + i) * 0.03125;
    dsamp += (b + d + f + h) * 0.0625;
    dsamp += (j + k + l + m) * 0.125;

    if (u_DoPrefilter == true) { dsamp = prefilter_threshold(dsamp); }

    dsamp      = max(dsamp, 0.0001f);
    downsample = vec4(dsamp, 1);
}
