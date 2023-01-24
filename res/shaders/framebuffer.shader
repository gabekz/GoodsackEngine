#shader vertex
#version 330 core

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoords;

out vec2 texCoords;

void main()
{
    gl_Position = vec4(inPos.x, inPos.y, 0.0, 1.0);
    texCoords = inTexCoords;
}

#shader fragment
#version 330 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D u_ScreenTexture;
uniform int u_Tonemapper = 1;
uniform float u_Exposure = 2.5;
uniform float u_Gamma = 2.2;
uniform bool u_GammaEnable = true;
uniform float u_MaxWhite = 1.0;

// -- Tonemappers

float luminance(vec3 v) { return dot(v, vec3(0.2126f, 0.7152f, 0.0722f)); }

vec3 reinhard(vec3 v) { return v / (1.0f + v); }

vec3 reinhard_jodie(vec3 v)
{
    float l = luminance(v);
    vec3 tv = v / (1.0f + v);
    return mix(v / (1.0f + l), tv, tv);
}

vec3 reinhard_extended(vec3 v, float max_white)
{
    vec3 numerator = v * (1.0f + (v / vec3(max_white * max_white)));
    return numerator / (1.0f + v);
}

vec3 aces_approx(vec3 x)
{
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 uncharted2_tonemap_partial(vec3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 uncharted2_filmic(vec3 v)
{
    float exposure_bias = 2.0f;
    vec3 curr = uncharted2_tonemap_partial(v * exposure_bias);

    vec3 W = vec3(11.2f);
    vec3 white_scale = vec3(1.0f) / uncharted2_tonemap_partial(W);
    return curr * white_scale;
}

//----------------------
void main()
{
    vec3 hdrColor = texture(u_ScreenTexture, texCoords).rgb;

    switch(u_Tonemapper) {
    case 0:
        hdrColor = reinhard(hdrColor);
        break;
    case 1:
        hdrColor = reinhard_jodie(hdrColor);
        break;
    case 2:
        hdrColor = reinhard_extended(hdrColor, u_MaxWhite);
        break;
    case 3:
        hdrColor = aces_approx(hdrColor);
        break;
    case 4:
        hdrColor = uncharted2_filmic(hdrColor);
        break;
    default:
        break;
    }

    vec3 result = vec3(1.0f) - exp(-hdrColor * u_Exposure);

    if(u_GammaEnable)
        result = pow(result, vec3(1.0 / u_Gamma));

    FragColor.rgb = result;

    //FragColor = vec4(result, 1.0);
    //FragColor = texture(u_ScreenTexture, texCoords);
    //FragColor = vec4(vec3(1.0 - texture(u_ScreenTexture, texCoords)), 1.0);
    //FragColor = kernel();
}
