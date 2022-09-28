#ifndef H_MODEL
#define H_MODEL

#include <util/sysdefs.h>

#include <core/api/opengl/glbuffer.h>
#include <core/texture.h>

typedef struct _model Model;

struct _model {
    VAO* vao;
    ui32 vertexCount;
    ui32 indicesCount;
    const char *modelPath;
};
#endif // H_MODEL
