#ifndef H_SKYBOX
#define H_SKYBOX

#include <core/api/opengl/opengl.h>
#include <core/texture/texture.h>

#include <core/shader/shader.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Skybox
{
    Texture *cubemap;
    Texture *hdrTexture;
    Texture *irradianceMap;
    VAO *vao;
    ShaderProgram *shader;
} Skybox;

Skybox *
skybox_create(Texture *cubemap);

void
skybox_draw(Skybox *self);

// HDR

Skybox *
skybox_hdr_create();
Texture *
skybox_hdr_projection(Skybox *skybox);

#ifdef __cplusplus
}
#endif

#endif // H_SKYBOX
