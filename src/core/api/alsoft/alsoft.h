#ifndef H_OPENAL
#define H_OPENAL

#include <AL/al.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int
openal_get_devices();

int
openal_init();

ALuint
openal_generate_source(const char *filepath);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_OPENAL
