#version 330 core

vec4 grayscale() {
    vec4 col = texture(u_ScreenTexture, texCoords);
    float avg = (col.r + col.g + col.b) / 3.0;
    col = vec4(avg, avg, avg, 1.0);
    return col;
}

vec4 kernel() {
    const float offset = 1.0 / 300.0;

    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right   
    );

    float sobelKernel[9] = float[](
        -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1
    );

    float blurKernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    );

    float randKernel[9] = float[](
        1, -1, -1,
        -1, 8, -1,
        -1, 1, -1
    );
    float edgeKernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );

    float kern[] = blurKernel;

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++) {
        sampleTex[i] = vec3(texture(u_ScreenTexture, texCoords.st + offsets[i]));
    }

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
        col += sampleTex[i] * kern[i];
    }
    return vec4(col, 1.0);
}
