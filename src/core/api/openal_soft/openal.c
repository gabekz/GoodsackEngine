#include "openal.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <stdlib.h>
#include <string.h>

#include <asset/import/loader_wav.h>
#include <util/logger.h>

int
openal_debug_callback()
{
    ALenum error = alGetError();
    if(error != AL_NO_ERROR) {
        switch(error)
        {
            case AL_INVALID_NAME:
                LOG_ERROR("[OpenAL] AL_INVALID_NAME: a bad name (ID) was given.");
                break;
            case AL_INVALID_ENUM:
                LOG_ERROR("[OpenAL] AL_INVALID_ENUM: an invalid enum value was passed.");
                break;
            case AL_INVALID_VALUE:
                LOG_ERROR("[OpenAL] AL_INVALID_ENUM: an invalid value was passed.");
                break;
            case AL_INVALID_OPERATION:
                LOG_ERROR("[OpenAL] AL_INVALID_OPERATION: the requested operation is not valid.");
                break;
            case AL_OUT_OF_MEMORY:
                LOG_ERROR("[OpenAL] AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory.");
                break;
            default:
                LOG_CRITICAL("[OpenAL] UNKOWN OpenAL-Soft Error! Error Code: %d", error);
                break;
        }
        return 0;
    }

    return 1;
}

static void
list_audio_devices(const ALCchar *devices)
{
    const ALCchar *device = devices, *next = devices + 1;
    size_t len = 0;

    LOG_INFO("Audio Devices List:");
    while(devices && *device != '\0' && next && *next != '\0') {
        LOG_INFO("Device: %s", device);
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
}

static ALenum file_to_al_format(ui16 channels, ui16 samples)
{
    int stereo = (channels > 1);

    switch(samples) {
        case 16:
            return (stereo) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
        case 8:
            return (stereo) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
        default:
            return -1;
    }

}

ALCdevice *
openal_get_device()
{
    ALCdevice *device;
    device = alcOpenDevice(NULL);

    if(!device) {
        LOG_CRITICAL("[OpenAL] Failed to fetch audio device!");
    }

    ALboolean enumeration;
    enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    if(!enumeration) {

    }
    else {
        list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
    }

    return device;
}

int openal_init()
{
    ALCdevice *device = openal_get_device();
    ALCcontext *context = alcCreateContext(device, NULL);

    if (!alcMakeContextCurrent(context)) {
        LOG_CRITICAL("[OpenAL] Failed to make context current");
        return 0;
    }

    // Defining and configuring the listener
    ALfloat listenerOrientation[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

    AL_CHECK(alListener3f(AL_POSITION, 0, 0, 1.0f));
    AL_CHECK(alListener3f(AL_VELOCITY, 0, 0, 0));
    AL_CHECK(alListenerfv(AL_ORIENTATION, listenerOrientation));

    openal_generate_source();

    return 1;
}

ALuint
openal_generate_source()
{
    ALuint source;
    AL_CHECK(alGenSources((ALuint)1, &source));

    AL_CHECK(alSourcef(source, AL_PITCH, 1));
    AL_CHECK(alSourcef(source, AL_GAIN, 1));

    AL_CHECK(alSource3f(source, AL_POSITION, 0, 0, 0));
    AL_CHECK(alSource3f(source, AL_VELOCITY, 0, 0, 0));

    AL_CHECK(alSourcei(source, AL_LOOPING, AL_FALSE));

    // Create buffer and load audio
    ALuint buffer = openal_buffer_create("../res/audio/test.wav");
    // Bind buffer
    AL_CHECK(alSourcei(source, AL_BUFFER, buffer));
    // Play audio source
    alSourcePlay(source);

    // cleanup

    return source;
}

ALuint
openal_buffer_create(const char *filePath)
{
    // Load .wav file
    AudioData *rawData = load_wav(filePath);
    ALenum format = file_to_al_format(rawData->channels, rawData->samples);

    ALuint buffer;
    AL_CHECK(alGenBuffers((ALuint)1, &buffer));
    AL_CHECK(alBufferData(buffer, AL_FORMAT_MONO16, rawData->data, rawData->samples * 2, rawData->samples));


    return buffer;
}

/*
ALuint
openal_buffer_create(ALenum format, ALvoid *data, ALsizei size, ALsizei samplerate)
{
    ALuint buffer;
    AL_CHECK(alGenBuffers((ALuint)1, &buffer));
    AL_CHECK(alBufferData(buffer, format, data, size, samplerate));
}
*/
