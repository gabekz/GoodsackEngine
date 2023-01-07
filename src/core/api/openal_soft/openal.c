#include "openal.h"

#include <stdlib.h>
#include <string.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <core/api/openal_soft/openal_buffer.h>
#include <core/api/openal_soft/openal_debug.h>

#include <util/logger.h>

/* static */

static void
ListAudioDevices(const ALCchar *devices)
{
    const ALCchar *device = devices, *next = devices + 1;
    size_t len = 0;

    LOG_INFO("Audio Devices List:");
    while (devices && *device != '\0' && next && *next != '\0') {
        LOG_INFO("Device: %s", device);
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
}

/* implementation */

ALCdevice *
openal_get_device()
{
    ALCdevice *device;
    device = alcOpenDevice(NULL);

    if (!device) { LOG_CRITICAL("[OpenAL] Failed to fetch audio device!"); }

    ALboolean enumeration;
    enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    if (!enumeration) {

    } else {
        ListAudioDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
    }

    return device;
}

int
openal_init()
{
    ALCdevice *device   = openal_get_device();
    ALCcontext *context = alcCreateContext(device, NULL);

    if (!alcMakeContextCurrent(context)) {
        LOG_CRITICAL("[OpenAL] Failed to make context current");
        return 0;
    }

    // Defining and configuring the listener
    ALfloat listenerOrientation[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};

    AL_CHECK(alListener3f(AL_POSITION, 0, 0, 1.0f));
    AL_CHECK(alListener3f(AL_VELOCITY, 0, 0, 0));
    AL_CHECK(alListenerfv(AL_ORIENTATION, listenerOrientation));

    // openal_generate_source();

    return 1;
}

ALuint
openal_generate_source(const char *filepath)
{
    ALuint source;
    AL_CHECK(alGenSources((ALuint)1, &source));

    AL_CHECK(alSourcef(source, AL_PITCH, 1));
    AL_CHECK(alSourcef(source, AL_GAIN, 1));

    AL_CHECK(alSource3f(source, AL_POSITION, 0, 0, 0));
    AL_CHECK(alSource3f(source, AL_VELOCITY, 0, 0, 0));

    AL_CHECK(alSourcei(source, AL_LOOPING, AL_FALSE));

    // Create buffer and load audio
    ALuint buffer = openal_buffer_create(filepath);
    // Bind buffer
    AL_CHECK(alSourcei(source, AL_BUFFER, buffer));
    // Play audio source
    alSourcePlay(source);

    // cleanup

    return source;
}
