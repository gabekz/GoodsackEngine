#ifndef H_SCENE
#define H_SCENE

/* scene steps::
 * 1) Initialize and set active camera
 * 2) Create Light information
 [Standard]
 * 3) Renderer tick [all logic updates + shader updates]
 * 4) Render [all meshes in meshList]
 [ECS]
 * 3) Process Systems
 */

#include <util/sysdefs.h>

typedef struct _scene Scene;

struct _scene
{
    ui32 id;

    ui32 meshC, lightC;

    struct _ecs *ecs;
};

#endif // H_SCENE
