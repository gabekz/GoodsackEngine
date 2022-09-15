#ifndef LOADER_OBJ_H
#define LOADER_OBJ_H

//#include "../glbuffer/glbuffer.h"
#include <glbuffer/glbuffer.h>
#include <util/sysdefs.h>
#include <model/mesh.h>

Model* load_obj(const char* path);

#endif /* LOADER_OBJ_H */
