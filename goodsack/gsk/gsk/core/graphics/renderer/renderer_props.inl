#ifndef __RENDERER_PROPS_INL__
#define __RENDERER_PROPS_INL__

#include "util/sysdefs.h"

//enum Tonemappers { Reinhard = 0, ACES };

// Renderer Properties
typedef struct RendererProps
{
    ui32 tonemapper;

    float exposure;
    float maxWhite;

    float gamma;
    int gammaEnable;

    ui32 msaaSamples;
    int msaaEnable;

    float vignetteAmount;
    float vignetteFalloff;

} RendererProps;

#endif // __RENDERER_PROPS_INL__