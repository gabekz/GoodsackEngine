/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_wav.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/logger.h"
#include "util/sysdefs.h"

#define LOG_LOADER_AUDIO FALSE

gsk_AudioData *
gsk_load_wav(const char *filepath)
{

    gsk_AudioData *ret = malloc(sizeof(gsk_AudioData));

    FILE *filePtr;
    char magic[5];
    s32 fileSize;
    s32 formatLength;
    s16 formatType;
    s16 numChannels;
    s32 sampleRate;
    s32 bytesPerSecond; // sampleRate * numChannels * bitsPerSample
    s16 blockAlign;     // numChannels * bitsPerSample
    s16 bitsPerSample;  // 16
    s32 dataSize;

    magic[4] = '\0';

    filePtr = fopen(filepath, "rb");

    if (filePtr == NULL)
    {
        LOG_ERROR("Failed to open file: %s", filepath);
        return NULL;
    }

    fread(magic, strlen("RIFF"), 1, filePtr);
    if (strcmp(magic, "RIFF"))
    {
        LOG_ERROR("First 4 bytes should be \"RIFF\", are \"%4s\"", magic);
    }

    fread(&fileSize, 4, 1, filePtr);

    fread(magic, 1, 4, filePtr);
    if (strcmp(magic, "WAVE")) { LOG_ERROR("Failed"); }

    fread(magic, 1, 4, filePtr);
    if (strcmp(magic, "fmt ")) { LOG_ERROR("Failed"); }

    fread(&formatLength, 4, 1, filePtr);

    fread(&formatType, 2, 1, filePtr);
    if (formatType != 1)
    {
        LOG_ERROR("%s format type should be 1, is %s", magic);
    }

    fread(&numChannels, 2, 1, filePtr);
    if (numChannels != 1)
    {
        LOG_ERROR("Number of channels exceeds 1, is %d", numChannels);
    }
    ret->numChannels = numChannels;

    fread(&sampleRate, 4, 1, filePtr);
    if (sampleRate != SAMPLING_RATE)
    {
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
    if (!strcmp(magic, "LIST"))
    {
        // WAV metadata
        u32 listSize;
        u16 listType;
        void *listData;

        fread(&listSize, 1, 4, filePtr);
        fread(&listType, 1, 4, filePtr);
        fread(&listData, listSize, 1, filePtr);
    }

    if (!strcmp(magic, "junk"))
    {
        s32 junk_size = 0;
        u8 odd_offset = (junk_size % 2 == 1) ? 1 : 0;

        fread(&junk_size, 1, 4, filePtr);
        fseek(filePtr, junk_size + odd_offset, SEEK_CUR);

        fread(magic, 1, 4, filePtr);
    }

    if (strcmp(magic, "data"))
    {
        LOG_ERROR("Failed to read 'data' string, magic value is %s", magic);
    }

    fread(&dataSize, 4, 1, filePtr);

    ret->data = malloc(dataSize);
    if (ret->data == NULL)
    {
        LOG_ERROR("Failed to allocate memory for audio data");
        free(ret->data);
    }

    if (fread(ret->data, 1, dataSize, filePtr) != dataSize)
    {
        LOG_ERROR("Failed to read data bytes");
    }
    ret->dataSize = dataSize;

#if LOG_LOADER_AUDIO
    LOG_PRINT("-----------------");
    LOG_TRACE("Loaded WAV File: %s", filepath);
    LOG_PRINT("Sampling rate is %d", SAMPLING_RATE, sampleRate);
    LOG_PRINT("Samples %d", bitsPerSample);
    LOG_PRINT("BPS %d", bytesPerSecond);
    LOG_PRINT("Block Align %d", blockAlign);
    LOG_PRINT("Data size is %d", dataSize);
    LOG_PRINT("-----------------");
#endif

    fclose(filePtr);
    return ret;
}
