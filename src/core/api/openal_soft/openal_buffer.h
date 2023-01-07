#ifndef H_OPENAL_BUFFER
#define H_OPENAL_BUFFER

#include <AL/al.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Creates an OpenAL audio buffer and fills the data immediately
 * by loading the specified .wav file.
 *
 * @param[in] filepath to the specified .wav file
 * @return buffer ID (ALuint)
 */
ALuint
openal_buffer_create(const char *filePath);

/**
 * Deletes the specified OpenAL audio buffer
 *
 * @param[in] buffer Id
 */
void
openal_buffer_cleanup(ALuint buffer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_OPENAL_BUFFER
