#ifndef H_LOADER_WAV
#define H_LOADER_WAV

#include <util/sysdefs.h>

#define SAMPLING_RATE   44100
#define CHUNK_SIZE      2000

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    ui32 channels, samples;
    ui16 *data;
} AudioData;

// 16-bit 1-Channel PCM .wav file loader.
// @return struct AudioData
AudioData *
load_wav(const char *filepath);

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#endif // H_LOADER_WAV
