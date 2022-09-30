#ifndef H_C_MESH
#define H_C_MESH

#include <model/model.h>
#include <model/material.h>

#include <core/ecs.h>

#define DRAW_ARRAYS     0x00
#define DRAW_ELEMENTS   0x01

#define CULL_DISABLED   0x10
#define CULL_CW         0x00
#define CULL_CCW        0x01
#define CULL_FORWARD    0x00
#define CULL_BACK       0x02

struct ComponentMesh {
    Material *material;
    const char *modelPath;
    Model *model;

    struct {
        ui16 renderMode: 1;
        ui16 drawMode: 2;
        ui16 cullMode: 3;
    } properties;

    ui32 vbo;
};

void s_draw_mesh_init(ECS *ecs);

#endif // H_C_MESH
