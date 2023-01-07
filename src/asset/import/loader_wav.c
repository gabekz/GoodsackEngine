#include "loader_wav.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/logger.h>
#include <util/sysdefs.h>

AudioData *
load_wav(const char *filepath)
{

    AudioData *ret = malloc(sizeof(AudioData));

    FILE *filePtr;
    char magic[4];
    si32 fileSize;
    si32 formatLength;
    si16 formatType;
    si16 numChannels;
    si32 sampleRate;
    si32 bytesPerSecond; // sampleRate * numChannels * bitsPerSample
    si16 blockAlign;     // numChannels * bitsPerSample
    si16 bitsPerSample;  // 16
    si32 dataSize;

    filePtr = fopen(filepath, "rb");
    if (filePtr == NULL) { LOG_ERROR("Failed to open file: %s", filepath); }

    fread(magic, 1, 4, filePtr);
    if (strcmp(magic, "RIFF")) {
        LOG_ERROR("First 4 bytes should be \"RIFF\", are \"%4s\"", magic);
    }

    fread(&fileSize, 4, 1, filePtr);

    fread(magic, 1, 4, filePtr);
    if (strcmp(magic, "WAVE")) { LOG_ERROR("Failed"); }

    fread(magic, 1, 4, filePtr);
    if (strcmp(magic, "fmt ")) { LOG_ERROR("Failed"); }

    fread(&formatLength, 4, 1, filePtr);

    fread(&formatType, 2, 1, filePtr);
    if (formatType != 1) {
        LOG_ERROR("%s format type should be 1, is %s", magic);
    }

    fread(&numChannels, 2, 1, filePtr);
    if (numChannels != 1) {
        LOG_ERROR("Number of channels exceeds 1, is %d", numChannels);
    }
    ret->numChannels = numChannels;

    fread(&sampleRate, 4, 1, filePtr);
    if (sampleRate != SAMPLING_RATE) {
        LOG_ERROR("Incorrect Sample Rate. Expected %d, is %d",
                  SAMPLING_RATE,
                  sampleRate);
    }
    ret->sampleRate = sampleRate;

    fread(&bytesPerSecond, 4, 1, filePtr);
    fread(&blockAlign, 2, 1, filePtr);
    fread(&bitsPerSample, 2, 1, filePtr);
    if (bitsPerSample != 16) { LOG_ERROR("Bits per sample should be 16"); }
    ret->samples = bitsPerSample;

    fread(magic, 1, 4, filePtr);
    if (!strcmp(magic, "LIST")) {
        // WAV metadata
        ui32 listSize;
        ui16 listType;
        void *listData;

        fread(&listSize, 1, 4, filePtr);
        fread(&listType, 1, 4, filePtr);
        fread(&listData, listSize, 1, filePtr);
    }
    if (strcmp(magic, "data")) {
        LOG_ERROR("Failed to read 'data' string, magic value is %s", magic);
    }

    fread(&dataSize, 4, 1, filePtr);

    ret->data = malloc(dataSize);
    if (ret->data == NULL) {
        LOG_ERROR("Failed to allocate memory for audio data");
        free(ret->data);
    }

    if (fread(ret->data, 1, dataSize, filePtr) != dataSize) {
        LOG_ERROR("Failed to read data bytes");
    }
    ret->dataSize = dataSize;

    LOG_PRINT("-----------------");
    LOG_INFO("Loaded WAV File: %s", filepath);
    LOG_PRINT("Sampling rate is %d", SAMPLING_RATE, sampleRate);
    LOG_PRINT("Samples %d", bitsPerSample);
    LOG_PRINT("BPS %d", bytesPerSecond);
    LOG_PRINT("Block Align %d", blockAlign);
    LOG_PRINT("Data size is %d", dataSize);
    LOG_PRINT("-----------------");

    fclose(filePtr);
    return ret;
}
