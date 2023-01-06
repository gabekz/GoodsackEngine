#ifndef H_OPENAL_DEBUG
#define H_OPENAL_DEBUG

#include <AL/al.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if SYS_DEBUG == SYS_ENABLED
#define AL_CHECK(x) do { x; openal_debug_callback(); } while(0)
#else
#define AL_CHECK(x) x;
#endif

int
openal_debug_callback();

int
openal_get_devices();

int
openal_init();

ALuint
openal_generate_source();

ALuint
openal_buffer_create(const char *filePath);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_OPENAL_DEBUG
