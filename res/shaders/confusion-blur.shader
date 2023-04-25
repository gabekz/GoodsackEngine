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

//---------------------------------------------------------------------------
#shader fragment
#version 430 core

layout(location = 0) out vec3 outColor;

in vec2 texCoords;

layout(binding = 0) uniform sampler2D t_Near;
layout(binding = 1) uniform sampler2D t_Far;

uniform float xs = 1280;
uniform float ys = 720; // texture resolution
uniform float r  = 5;   // blur radius

vec4
gaussian(sampler2D txr, vec2 pos)
{
    float x, y, xx, yy, rr = r * r, dx, dy, w, w0;
    w0 = 0.3780 / pow(r, 1.975);
    vec2 p;
    vec4 col = vec4(0.0, 0.0, 0.0, 0.0);
    for (dx = 1.0 / xs, x = -r, p.x = 0.5 + (pos.x * 0.5) + (x * dx); x <= r;
         x++, p.x += dx) {
        xx = x * x;
        for (dy = 1.0 / ys, y = -r, p.y = 0.5 + (pos.y * 0.5) + (y * dy);
             y <= r;
             y++, p.y += dy) {
            yy = y * y;
            if (xx + yy <= rr) {
                w = w0 * exp((-xx - yy) / (2.0 * rr));
                col += texture2D(txr, p) * w;
            }
        }
    }
    return col;
}

void
main()
{
    // outColor = gaussian(t_Far, texCoords).rgb;
    outColor = texture(t_Far, texCoords).rgb;
}