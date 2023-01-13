#ifndef H_SKYBOX
#define H_SKYBOX

#include <core/api/opengl/opengl.h>
#include <core/texture/texture.h>
#include <ecs/ecs.h>

struct Skybox
{
    Texture *cubemap;
    VAO *vao;
};

#endif // H_SKYBOX
