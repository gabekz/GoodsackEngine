#include "primitives.h"

#include <stdlib.h>

/*
static void* fillData(unsigned int size, unsigned int dataSize, void* dataIn) {
    void* ptr = malloc(size * dataSize);
    for(int i = 0; i < size; i++) {
        ptr[i] = dataIn[i];
    }
    return ptr;
}
*/

// * ----------- Rect -------------* //
float* prim_vert_rect() {

    float rectPoints[] = {
        // Coords   // texCoords
         1.0f, -1.0f,    1.0f, 0.0f,
        -1.0f, -1.0f,    0.0f, 0.0f,
        -1.0f,  1.0f,    0.0f, 1.0f,
    
         1.0f,  1.0f,     1.0f, 1.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
        -1.0f,  1.0f,     0.0f, 1.0f
    };
    
        unsigned int size = (2 * 3) * 4;
        float *ret = malloc(size * sizeof(float));
        for(int i = 0; i <= size; i++) {
            ret[i] = rectPoints[i];
        }
        return ret;
}



// * ----------- Plane -------------* //

float* prim_vert_plane() {

    float points[] = {
//  position        texture-uv
    -0.5f, -0.5f,   0.0f, 0.0f, 
     0.5f, -0.5f,   1.0f, 0.0f, 
     0.5f,  0.5f,   1.0f, 1.0f, 
    -0.5f,  0.5f,   0.0f, 1.0f  
    };

    //void *ret = (float *)fillData(16, sizeof(float), points);
    unsigned int size = (2 * 4) + (2 * 4);
    float *ret = malloc(16 * sizeof(float));
    for(int i = 0; i < 16; i++) {
        ret[i] = points[i];
    }
    return ret;
}

// * ----------- Pyramid -------------* //

float* prim_vert_pyramid() {

    float points[] = {
  // position              texture-uv
    -0.5f,  0.0f,  0.5f,   0.0f, 0.0f, 
    -0.5f,  0.0f, -0.5f,   5.0f, 0.0f, 
     0.5f,  0.0f, -0.5f,   0.0f, 0.0f, 
     0.5f,  0.0f,  0.5f,   5.0f, 0.0f,
     0.0f,  0.8f,  0.0f,   2.5f, 5.0f  
    };

    unsigned int size = (3 * 5) + (2 * 5);

    float *ret = malloc(size * sizeof(float));
    for(int i = 0; i < size; i++) {
        ret[i] = points[i];
    }
    return ret;

}

float* prim_vert_norm_pyramid () {
    float points[] =
{ //     COORDINATES     /  TexCoord   /        NORMALS       //
	-0.5f, 0.0f,  0.5f,     0.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side
	-0.5f, 0.0f, -0.5f,     0.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
	 0.5f, 0.0f, -0.5f,     5.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
	 0.5f, 0.0f,  0.5f,     5.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side

	-0.5f, 0.0f,  0.5f,     0.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
	-0.5f, 0.0f, -0.5f,     5.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,     -0.8f, 0.5f,  0.0f, // Left Side

	-0.5f, 0.0f, -0.5f,     5.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
	 0.5f, 0.0f, -0.5f,     0.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,      0.0f, 0.5f, -0.8f, // Non-facing side

	 0.5f, 0.0f, -0.5f,     0.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
	 0.5f, 0.0f,  0.5f,     5.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,      0.8f, 0.5f,  0.0f, // Right side

	 0.5f, 0.0f,  0.5f,     5.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
	-0.5f, 0.0f,  0.5f,     0.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,      0.0f, 0.5f,  0.8f  // Facing side
};

    unsigned int size = (3 * 2 * 3 * 16);

    float *ret = malloc(size * sizeof(float));
    for(int i = 0; i < size; i++) {
        ret[i] = points[i];
    }
    return ret;

}

float* prim_norm_pyramid() {

    float points[] = {
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,

        -0.8f, -0.5f, 0.0f,
        -0.8f, -0.5f, 0.0f,
        -0.8f, 0.5f, 0.0f,

        0.0f, 0.5f, -0.8f,
        0.0f, 0.5f, -0.8f,
        0.0f, 0.5f, -0.8f,

        0.8f, 0.5f, 0.0f,
        0.8f, 0.5f, 0.0f,
        0.8f, 0.5f, 0.0f,

        0.0f, 0.5f, 0.8f,
        0.0f, 0.5f, 0.8f,
        0.0f, 0.5f, 0.8f
    };

    unsigned int size = 3 * 4 * 4;

    float *ret = malloc(size * sizeof(float));
    for(int i = 0; i < size; i++) {
        ret[i] = points[i];
    }
    return ret;
}


// * ----------- Cube -------------* //

float* prim_vert_cube(float scale) {

    float points[] = {
       -1.0f,  -1.0f,  1.0f,
       -1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f,  1.0f,
       -1.0f,   1.0f,  1.0f,
       -1.0f,   1.0f, -1.0f,
        1.0f,   1.0f, -1.0f,
        1.0f,   1.0f,  1.0f
    };

    unsigned int size = 3 * 8;

    float *ret = malloc(size * sizeof(float));
    for(int i = 0; i < size; i++) {
        ret[i] = points[i] * scale;
    }
    return ret;
}
