#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#define PRIMITIVE_PLANE     0xAB00
#define PRIMITIVE_CUBE      0xAB01
#define PRIMITIVE_PYRAMID   0xAB02

#define PRIMITIVE_SIZE_PLANE    (3 * 8 * sizeof(float))
#define PRIMITIVE_SIZE_CUBE     (3 * 8 * sizeof(float))
#define PRIMITIVE_SIZE_PYRAMID  (3 * 8 * sizeof(float))

void primitive_pyramid(float* out);
float* prim_vert_rect();
float* prim_vert_plane();

float* prim_vert_pyramid();
float* prim_norm_pyramid();
float* prim_vert_norm_pyramid(); 

float* prim_vert_cube(float scale);

#endif // PRIMITIVES_H
