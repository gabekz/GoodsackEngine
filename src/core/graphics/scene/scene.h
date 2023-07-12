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

#include <core/graphics/lighting/skybox.h>
#include <util/sysdefs.h>

typedef struct Scene_t
{
    ui32 id, meshC, lightC;

    struct _ecs *ecs;

    Skybox *skybox;
    ui16 has_skybox;
} Scene;

#endif // H_SCENE
