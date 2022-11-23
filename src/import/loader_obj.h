#ifndef LOADER_OBJ_H
#define LOADER_OBJ_H

//#include "../glbuffer/glbuffer.h"
#include <core/api/opengl/glbuffer.h>
#include <util/sysdefs.h>
#include <model/model.h>

Model* load_obj(const char* path, float scale);

#endif /* LOADER_OBJ_H */
