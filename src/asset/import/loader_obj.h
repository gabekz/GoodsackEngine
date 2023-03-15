#ifndef LOADER_OBJ_H
#define LOADER_OBJ_H

// #include "../glbuffer/glbuffer.h"
#include <core/drivers/opengl/opengl.h>
#include <core/graphics/mesh/mesh.h>
#include <util/sysdefs.h>

MeshData *
load_obj(const char *path, float scale);

#endif /* LOADER_OBJ_H */
