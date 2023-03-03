#ifndef H_LIGHTING
#define H_LIGHTING

#include <cglm/cglm.h>
#include <cglm/struct.h>

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/shader/shader.h>
#include <core/graphics/texture/texture.h>

typedef struct _light Light;

typedef enum e_lightType { Directional = 0, Point = 1, Spot = 2 } LightType;

struct _light
{
    float *position;
    float *color;
    LightType type;
};

#ifdef __cplusplus
extern "C" {
#endif

Light *
light_create(float *position, float *color, LightType type);
void
lighting_initialize(float *lightPos, float *lightColor);

#ifdef __cplusplus
}
#endif

#endif // H_LIGHTING