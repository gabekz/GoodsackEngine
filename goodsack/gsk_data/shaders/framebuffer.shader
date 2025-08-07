#shader vertex
#version 420 core

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
#version 420 core

in vec2 texCoords;

layout(binding = 0) uniform sampler2D u_ScreenTexture;
layout(binding = 1) uniform sampler2D u_BloomTexture;

uniform int u_Tonemapper   = 1;
uniform float u_Exposure   = 2.5;
uniform float u_Gamma      = 2.2;
uniform bool u_GammaEnable = true;
uniform float u_MaxWhite   = 1.0;

uniform float u_VignetteAmount  = 0.5;
uniform float u_VignetteFalloff = 0.5;
uniform vec3 u_VignetteColor    = vec3(1);

uniform float u_BloomIntensity = 1.0;

out vec4 FragColor;

// -- ACES TESTING
const mat3 ACESInputMat = mat3(
  // Row0 => (0.59719, 0.35458, 0.04823)
  0.59719,
  0.07600,
  0.02840, // <-- these will end up as column0
  0.35458,
  0.90834,
  0.13383, // <-- column1
  0.04823,
  0.01566,
  0.83777 // <-- column2
);

// ACES Output matrix (ODT_SAT => XYZ => D60_2_D65 => sRGB)
const mat3 ACESOutputMat = mat3(
  // Row0 => ( 1.60475, -0.53108, -0.07367)
  1.60475,
  -0.10208,
  -0.00327,
  -0.53108,
  1.10813,
  -0.07276,
  -0.07367,
  -0.00605,
  1.07602);

vec3
RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3
aces_fitted(vec3 color)
{
    // Matrix multiply
    color = ACESInputMat * color;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    // Output matrix multiply
    color = ACESOutputMat * color;

    color = clamp(color, 0.0, 1.0);

    return color;
}

// -- Tonemappers

float
luminance(vec3 v)
{
    return dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
}

vec3
reinhard(vec3 v)
{
    return v / (1.0f + v);
}

vec3
reinhard_jodie(vec3 v)
{
    float l = luminance(v);
    vec3 tv = v / (1.0f + v);
    return mix(v / (1.0f + l), tv, tv);
}

vec3
reinhard_extended(vec3 v, float max_white)
{
    vec3 numerator = v * (1.0f + (v / vec3(max_white * max_white)));
    return numerator / (1.0f + v);
}

vec3
aces_approx(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3
filmic_partial(vec3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3
filmic_full(vec3 v)
{
    float exposure_bias = 2.0f;
    vec3 curr           = filmic_partial(v * exposure_bias);

    vec3 W           = vec3(11.2f);
    vec3 white_scale = vec3(1.0f) / filmic_partial(W);
    return curr * white_scale;
}

//----------------------
vec3
sample_tonemap_gamma(vec3 col_in)
{
    // vec3 col_rgb = texture(tex_in, texCoords).rgb;
    vec3 col_rgb = col_in;
    vec3 ret     = col_rgb;

    switch (u_Tonemapper)
    {
    case 0: ret = reinhard(col_rgb); break;
    case 1: ret = reinhard_jodie(col_rgb); break;
    case 2: ret = reinhard_extended(col_rgb, u_MaxWhite); break;
    case 3: ret = filmic_full(col_rgb); break;
    case 4: ret = aces_approx(col_rgb); break;
    case 5: ret = aces_fitted(col_rgb); break;
    default: break;
    }

    ret = vec3(1.0f) - exp(-ret * u_Exposure);

    if (u_GammaEnable) { ret = pow(ret, vec3(1.0 / u_Gamma)); }

    return ret;
}

//----------------------
void
main()
{
    // vec3 pixel_rgb
    //  (sample_tonemap_gamma(u_ScreenTexture) +
    //   (sample_tonemap_gamma(u_BloomTexture) * u_BloomIntensity));
    vec3 pixel_rgb =
      (texture(u_ScreenTexture, texCoords).rgb +
       (texture(u_BloomTexture, texCoords).rgb * u_BloomIntensity));

    pixel_rgb = sample_tonemap_gamma(pixel_rgb);

    // Vignette
    float vignette_factor = smoothstep(
      0.8,
      u_VignetteFalloff * 0.799,
      distance(texCoords, vec2(0.5)) * (u_VignetteAmount + u_VignetteFalloff));

    vec3 vignette_color = mix(u_VignetteColor, pixel_rgb, vignette_factor);
    pixel_rgb           = vignette_color;

    // Kernel
    // FragColor = vec4(result, 1.0);
    // FragColor = texture(u_ScreenTexture, texCoords);
    // FragColor = vec4(vec3(1.0 - texture(u_ScreenTexture, texCoords)), 1.0);
    // FragColor = kernel();

    // Output
    FragColor.rgb = pixel_rgb;
    // vec3 col_rgb  = texture(u_ScreenTexture, texCoords).rgb;
    // FragColor.rgb = vec3(col_rgb.r);
}
