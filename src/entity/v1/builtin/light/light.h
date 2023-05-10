#ifndef H_C_LIGHT
#define H_C_LIGHT

#include <util/maths.h>
#include <util/sysdefs.h>

#include <entity/ecs.h>

#if !(USING_GENERATED_COMPONENTS)
struct ComponentLight
{
    vec4 color;
    ui32 type;
};
#endif

#endif // H_C_LIGHT
