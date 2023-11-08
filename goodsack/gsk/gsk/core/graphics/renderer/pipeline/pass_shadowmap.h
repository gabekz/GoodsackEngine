#ifndef H_PASS_SHADOWMAP
#define H_PASS_SHADOWMAP

#include <core/graphics/material/material.h>
#include <util/maths.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct ShadowmapOptions
{
    float nearPlane, farPlane;
    float camSize;

    float normalBiasMin, normalBiasMax;
    si32 pcfSamples;
} ShadowmapOptions;

/**
 * Creates all data needed for the shadowmap
 */
void
shadowmap_init();

/**
 * Binds shadowmap framebuffer and textures
 */
void
shadowmap_bind();

/**
 * Update shadowmap projection-properties
 * @param[in] directional-light position
 */
void
shadowmap_update(vec3 lightPosition, ShadowmapOptions options);

void
shadowmap_bind_texture();
Material *
shadowmap_getMaterial();
ui32
shadowmap_getTexture();
vec4 *
shadowmap_getMatrix();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
