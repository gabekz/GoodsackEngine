#include "alsoft_debug.h"

#include <stdlib.h>
#include <string.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <asset/import/loader_wav.h>
#include <util/logger.h>

int
openal_debug_callback()
{
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        switch (error) {
        case AL_INVALID_NAME:
            LOG_ERROR("[OpenAL] AL_INVALID_NAME: a bad name (ID) was given.");
            break;
        case AL_INVALID_ENUM:
            LOG_ERROR(
              "[OpenAL] AL_INVALID_ENUM: an invalid enum value was passed.");
            break;
        case AL_INVALID_VALUE:
            LOG_ERROR("[OpenAL] AL_INVALID_ENUM: an invalid value was passed.");
            break;
        case AL_INVALID_OPERATION:
            LOG_ERROR("[OpenAL] AL_INVALID_OPERATION: the requested operation "
                      "is not valid.");
            break;
        case AL_OUT_OF_MEMORY:
            LOG_ERROR("[OpenAL] AL_OUT_OF_MEMORY: the requested operation "
                      "resulted in OpenAL running out of memory.");
            break;
        default:
            LOG_CRITICAL("[OpenAL] UNKOWN OpenAL-Soft Error! Error Code: %d",
                         error);
            break;
        }
        return 0;
    }

    return 1;
}
