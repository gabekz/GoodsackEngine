#include "audio_listener.h"

#include <AL/al.h>
#include <core/drivers/alsoft/alsoft.h>
#include <core/drivers/alsoft/alsoft_debug.h>

#include <entity/v1/builtin/transform/transform.h>

static void
init(Entity e)
{
    if (!(ecs_has(e, C_AUDIOLISTENER))) return;

    // TODO: Move initialization out of here. Should be initializing
    // Audio device in main program.
    openal_init();
}

static void
update(Entity e)
{
    if (!(ecs_has(e, C_AUDIOLISTENER))) return;

    // AL_CHECK(alListener3f(AL_POSITION, 0, 0, 1.0f));
    // AL_CHECK(alListener3f(AL_VELOCITY, 0, 0, 0));

    if ((ecs_has(e, C_TRANSFORM))) {
        struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
        AL_CHECK(alListener3f(AL_POSITION,
                              transform->position[0],
                              transform->position[1],
                              transform->position[2]));
        AL_CHECK(alListener3f(AL_VELOCITY, 0, 0, 0));

        /* TODO: Listener orientation
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
        */
    }
}

void
s_audio_listener_init(ECS *ecs)
{
    //_ECS_DECL_COMPONENT(
    // ecs, C_AUDIO_LISTENER, sizeof(struct ComponentAudioListener));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init    = (ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = NULL,
                          .update  = (ECSSubscriber)update,
                        }));
}
