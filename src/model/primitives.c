#include "primitives.h"

#include <stdlib.h>
#include <util/sysdefs.h>

Model *
primitive_model_create(ui32 shape, float scale)
{

    Model *ret = malloc(sizeof(Model));

    switch (shape) {
    case PRIMITIVE_CUBE:
        ret->modelData->vertexCount  = PRIM_SIZ_V_CUBE;
        ret->modelData->indicesCount = PRIM_SIZ_I_CUBE;
        break;
    }

    return ret;
}

//  * ----------- Rect -------------* //
float *
prim_vert_rect()
{

    unsigned int size = PRIM_SIZ_V_PLANE;
    float *ret        = malloc(size * sizeof(float));
    for (int i = 0; i < size; i++) {
        ret[i] = PRIM_ARR_V_PLANE[i];
    }
    return ret;
}
