#ifndef H_SHADER
#define H_SHADER

#include <util/sysdefs.h>
#include <util/gfx.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _shaderProgram   ShaderProgram;
typedef struct _shaderSource    ShaderSource;

struct _shaderProgram {
    ui32 id;
    ShaderSource *shaderSource;
};

struct _shaderSource {
   char *shaderVertex, *shaderFragment;
};

ShaderProgram *shader_create_program(const char *path);
void shader_use(ShaderProgram *shader);
void shader_uniform(ShaderProgram *shader, ui32 type, void* data);

#if __cplusplus
}
#endif  // __cplusplus

#endif // H_SHADER
