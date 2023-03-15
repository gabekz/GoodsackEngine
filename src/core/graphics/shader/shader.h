#ifndef H_SHADER
#define H_SHADER

#include <util/gfx.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _shaderProgram ShaderProgram;
typedef struct _shaderSource ShaderSource;

typedef enum UniformType {
    UNIFORM_1f = 0,
    UNIFORM_2f,
    UNIFORM_3f,
    UNIFORM_4f,

    UNIFORM_1i,
    UNIFORM_2i,
    UNIFORM_3i,
    UNIFORM_4i,

    UNIFORM_1ui,
    UNIFORM_2ui,
    UNIFORM_3ui,
    UNIFORM_4ui,

    UNIFORM_1fv,
    UNIFORM_2fv,
    UNIFORM_3fv,
    UNIFORM_4fv,

    UNIFORM_1iv,
    UNIFORM_2iv,
    UNIFORM_3iv,
    UNIFORM_4iv,

} UniformType;

struct _shaderProgram
{
    ui32 id;
    ShaderSource *shaderSource;
};

struct _shaderSource
{
    char *shaderVertex, *shaderFragment, *shaderCompute;
};

ShaderProgram *
shader_create_program(const char *path);

ShaderProgram *
shader_create_compute_program(const char *path);

void
shader_use(ShaderProgram *shader);

void
shader_uniform(ShaderProgram *shader,
               const char *uniform,
               ui32 type,
               void *data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_SHADER
