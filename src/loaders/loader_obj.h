#ifndef LOADER_OBJ_H
#define LOADER_OBJ_H

//#include "../glbuffer/glbuffer.h"
#include <core/opengl/glbuffer.h>
#include <util/sysdefs.h>
#include <model/mesh.h>

Model* load_obj(const char* path, float scale);

#endif /* LOADER_OBJ_H */
