#include "primitives.h"

#include <stdlib.h>
#include <util/sysdefs.h>

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



#if 0
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

#endif
// * ----------- Cube -------------* //

float* prim_vert_cube(float scale) {

    float pointsT[] = {
        -1.0f, -1.0f, 1.0f, //Vertex 0
        1.0f, -1.0f, 1.0f,  //v1
        -1.0f, 1.0f, 1.0f,  //v2
        1.0f, 1.0f, 1.0f,   //v3

        1.0f, -1.0f, 1.0f,  //...
        1.0f, -1.0f, -1.0f,         
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,

        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,            
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,         
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,         
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,           
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f
    };

    unsigned int size = 3 * 4 * 6;
    float *ret = malloc(size * sizeof(float));
    for(int i = 0; i <= size; i++) {
        ret[i] = pointsT[i] * scale;
    }
    return ret;
}

float* prim_vert_cube_elements(float scale) {

    float cubeVertices[] = {
    -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    -1.0,  1.0,  1.0,
    1.0,  1.0,  1.0,
    -1.0, -1.0, -1.0,
    1.0, -1.0, -1.0,
    -1.0,  1.0, -1.0,
    1.0,  1.0, -1.0,
};

    ui32 cubeIndices[] = {
    0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
};

    unsigned int size = 3 * 8;
    float *ret = malloc(size * sizeof(float));
    for(int i = 0; i <= size; i++) {
        ret[i] = cubeVertices[i] * scale;
    }
    return ret;
}
