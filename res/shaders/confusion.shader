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

layout(location = 0) out vec3 outNear;
layout(location = 1) out vec3 outFar;

in vec2 texCoords;
layout(binding = 0) uniform sampler2D t_gPos;
layout(binding = 1) uniform sampler2D t_Screen; // screen-texture

void
main()
{
    float F = 0.4; // Focal Length
    float N = 0.5; // Aperture
    float C = 25;  // Diameter of CoC
    float H = F + (pow(F, 2) / (N * C));

    float S     = abs(texture(t_gPos, texCoords).z);
    float dNear = (H * S) / (H - S);
    float dFar  = (H * S) / (H + S);

    vec2 planes;
    planes.r = dNear;
    planes.g = dFar;

    outNear.rgb = vec3(planes.r);
    outFar.rgb  = vec3(planes.g);

    // screen texture stuff
}