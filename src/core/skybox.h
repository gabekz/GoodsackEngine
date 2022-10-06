#ifndef H_SKYBOX
#define H_SKYBOX

#include <core/ecs.h>
#include <core/api/opengl/glbuffer.h>
#include <core/texture.h>

struct Skybox {
    Texture *cubemap;
    VAO *vao;
};

#endif // H_SKYBOX
