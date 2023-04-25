#ifndef H_SKYBOX
#define H_SKYBOX

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/texture/texture.h>

#include <core/graphics/shader/shader.h>

#define HDR_DRAW_IRRADIANCE 0

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Skybox
{
    Texture *cubemap;
    Texture *hdrTexture;
    Texture *irradianceMap;
    Texture *prefilterMap;
    Texture *brdfLUTTexture;
    VAO *vao;
    ShaderProgram *shader;
} Skybox;

Skybox *
skybox_create(Texture *cubemap);

void
skybox_draw(Skybox *self);

// HDR

Skybox *
skybox_hdr_create(const char *hdrPath);
Texture *
skybox_hdr_projection(Skybox *skybox);

#ifdef __cplusplus
}
#endif

#endif // H_SKYBOX
