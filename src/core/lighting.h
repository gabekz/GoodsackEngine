#ifndef H_LIGHTING
#define H_LIGHTING

#include <cglm/cglm.h>
#include <cglm/struct.h>

#include <core/api/opengl/glbuffer.h>
#include <core/texture.h>
#include <core/shader.h>

typedef struct _light Light;
typedef struct _skybox Skybox;

typedef enum e_lightType { Directional = 0, Point = 1, Spot = 2 } LightType;

struct _light {
    float* position;
    float* color;
    LightType type;
};

struct _skybox {
    Texture *cubemap;
    VAO *vao;
    ShaderProgram *shader;
};

Light *light_create(float *position, float *color, LightType type);
void lighting_initialize(float *lightPos, float *lightColor);

Skybox *skybox_create(Texture *cubemap);
void skybox_draw(Skybox *self);

#endif // H_LIGHTING
