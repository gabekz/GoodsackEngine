#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <util/sysdefs.h>
#include "../mesh.h"

#define PRIMITIVE_PLANE     0xAB00
#define PRIMITIVE_CUBE      0xAB01
#define PRIMITIVE_PYRAMID   0xAB02

/*
         ------- Plane

//   Position        UV Coords     */
#define PRIM_ARR_V_PLANE (float[]){ \
     1.0f, -1.0f,   1.0f, 0.0f,     \
    -1.0f, -1.0f,   0.0f, 0.0f,     \
    -1.0f,  1.0f,   0.0f, 1.0f,     \
                                    \
     1.0f,  1.0f,   1.0f, 1.0f,     \
     1.0f, -1.0f,   1.0f, 0.0f,     \
    -1.0f,  1.0f,   0.0f, 1.0f}
#define PRIM_SIZ_V_PLANE    24

/*
         ------- Cube

//      Position        */
#define PRIM_SIZ_V_CUBE 24
#define PRIM_ARR_V_CUBE (float[]){ \
    -1.0, -1.0,  1.0,   \
     1.0, -1.0,  1.0,   \
    -1.0,  1.0,  1.0,   \
     1.0,  1.0,  1.0,   \
    -1.0, -1.0, -1.0,   \
     1.0, -1.0, -1.0,   \
    -1.0,  1.0, -1.0,   \
     1.0,  1.0, -1.0}

#define PRIM_SIZ_I_CUBE 14
#define PRIM_ARR_I_CUBE (ui32[]){ \
    0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1}

/*
         ------- Pyramid

//   Position               UV Coords       Normals             */
#define PRIM_ARR_V_PYRAMID (float[]){                           \
	-0.5f, 0.0f,  0.5f,     0.0f, 0.0f,      0.0f, -1.0f, 0.0f, \
	-0.5f, 0.0f, -0.5f,     0.0f, 5.0f,      0.0f, -1.0f, 0.0f, \
	 0.5f, 0.0f, -0.5f,     5.0f, 5.0f,      0.0f, -1.0f, 0.0f, \
	 0.5f, 0.0f,  0.5f,     5.0f, 0.0f,      0.0f, -1.0f, 0.0f, \
    \
	-0.5f, 0.0f,  0.5f,     0.0f, 0.0f,     -0.8f, 0.5f,  0.0f, \
	-0.5f, 0.0f, -0.5f,     5.0f, 0.0f,     -0.8f, 0.5f,  0.0f, \
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,     -0.8f, 0.5f,  0.0f, \
    \
	-0.5f, 0.0f, -0.5f,     5.0f, 0.0f,      0.0f, 0.5f, -0.8f, \
	 0.5f, 0.0f, -0.5f,     0.0f, 0.0f,      0.0f, 0.5f, -0.8f, \
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,      0.0f, 0.5f, -0.8f, \
    \
	 0.5f, 0.0f, -0.5f,     0.0f, 0.0f,      0.8f, 0.5f,  0.0f, \
	 0.5f, 0.0f,  0.5f,     5.0f, 0.0f,      0.8f, 0.5f,  0.0f, \
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,      0.8f, 0.5f,  0.0f, \
    \
	 0.5f, 0.0f,  0.5f,     5.0f, 0.0f,      0.0f, 0.5f,  0.8f, \
	-0.5f, 0.0f,  0.5f,     0.0f, 0.0f,      0.0f, 0.5f,  0.8f, \
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,      0.0f, 0.5f,  0.8f}
#define PRIM_SIZ_V_PYRAMID  288
#define PRIM_SIZ_I_PYRAMID  18
#define PRIM_ARR_I_PYRAMID (ui32[]){ \
        0, 1, 2, \
	    0, 2, 3, \
	    4, 6, 5, \
	    7, 9, 8, \
	    10, 12, 11, \
	    13, 15, 14}

Model *primitive_model_create(ui32 shape, float scale);

void primitive_pyramid(float* out);
float* prim_vert_rect();

#endif // PRIMITIVES_H
