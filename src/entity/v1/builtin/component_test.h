#ifndef H_COMPONENT_TEST
#define H_COMPONENT_TEST

#include <entity/ecsdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(USING_GENERATED_COMPONENTS)
typedef struct ComponentTest
{
    int movement_increment;
    float rotation_speed;
} ComponentTest;
#endif

#ifdef __cplusplus
}
#endif

#endif // H_COMPONENT_TEST