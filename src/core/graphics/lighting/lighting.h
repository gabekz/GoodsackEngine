#ifndef H_LIGHTING
#define H_LIGHTING

#include <cglm/cglm.h>
#include <cglm/struct.h>

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/shader/shader.h>
#include <core/graphics/texture/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum lightType_t { Directional = 0, Point = 1, Spot = 2 } LightType;

typedef struct light_t
{
    vec3 position;
    vec4 color;
    LightType type;
    float strength;

    ui32 ubo;
} Light;

Light *
lighting_initialize(vec3 lightPos, vec4 lightColor);
void
lighting_update(Light *light, vec3 lightPos, vec4 lightColor);

#ifdef __cplusplus
}
#endif

#endif // H_LIGHTING
