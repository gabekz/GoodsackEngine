#include "audio_listener.h"

#include <AL/al.h>
#include <core/api/alsoft/alsoft.h>
#include <core/api/alsoft/alsoft_debug.h>

#include <components/camera/camera.h>

static void
init(Entity e)
{
    if (!(ecs_has(e, C_AUDIO_LISTENER))) return;

    // TODO: Move initialization out of here. Should be initializing
    // Audio device in main program.
    openal_init();
}

static void
update(Entity e)
{
    if (!(ecs_has(e, C_AUDIO_LISTENER))) return;

    // AL_CHECK(alListener3f(AL_POSITION, 0, 0, 1.0f));
    // AL_CHECK(alListener3f(AL_VELOCITY, 0, 0, 0));

    if ((ecs_has(e, C_CAMERA))) { // TODO: Move Camera pos to transform
        struct ComponentCamera *camera = ecs_get(e, C_CAMERA);
        AL_CHECK(alListener3f(AL_POSITION,
                              camera->position[0],
                              camera->position[1],
                              camera->position[2]));
        AL_CHECK(alListener3f(AL_VELOCITY, 0, 0, 0));

        ALfloat listenerOrientation[] = {
          // View
          camera->center[0],
          camera->center[1],
          camera->center[2],
          // Axis-Up
          camera->axisUp[0],
          camera->axisUp[1],
          camera->axisUp[2],
        };
    }
}

void
s_audio_listener_init(ECS *ecs)
{
    ecs_component_register(
      ecs, C_AUDIO_LISTENER, sizeof(struct ComponentAudioListener));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init    = (ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = NULL,
                          .update  = (ECSSubscriber)update,
                        }));
}
