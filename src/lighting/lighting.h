#ifndef H_LIGHTING
#define H_LIGHTING

#include <cglm/cglm.h>
#include <cglm/struct.h>

typedef struct _light Light;
typedef enum e_lightType LightType;

enum e_lightType { Directional = 0, Point = 1, Spot = 2 };

struct _light {
    float* position;
    float* color;
    LightType type;
};

Light *light_create(float *position, float *color, LightType type);
void lighting_initialize(float *lightPos, float *lightColor);

#endif // H_LIGHTING
