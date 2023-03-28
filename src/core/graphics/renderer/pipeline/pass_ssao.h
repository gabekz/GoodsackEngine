#ifndef H_PASS_SSAO
#define H_PASS_SSAO

#include <util/sysdefs.h>

typedef struct SsaoOptions
{
    float strength, bias, radius;
    int kernelSize;

} SsaoOptions;

void
pass_ssao_init();

void
pass_ssao_bind();

ui32
pass_ssao_getOutputTextureId();

#endif // H_PASS_SSAO