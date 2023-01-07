#ifndef H_LOADER_WAV
#define H_LOADER_WAV

#include <util/sysdefs.h>

#define SAMPLING_RATE 44100

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct
{
    si32 sampleRate;
    ui32 numChannels, samples, dataSize;
    ui16 *data;
} AudioData;

// WAV file loader.
// @return struct AudioData
AudioData *
load_wav(const char *filepath);

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#endif // H_LOADER_WAV
