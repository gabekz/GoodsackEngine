/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "parse_image.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "asset/assetdefs.h"

#include "util/logger.h"

gsk_AssetBlob
parse_image(const char *path)
{
    gsk_AssetBlob ret = {.p_buffer = NULL, .buffer_len = 0};

    // find the location on disk
    char *buffer = 0;
    long length;
    FILE *f = fopen(path, "rb");

    if (f == NULL)
    {
        LOG_ERROR("Failed open file at %s", path);
        return ret;
    }

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(length);
    if (buffer) { fread(buffer, 1, length, f); }

    fclose(f);

    if (buffer == NULL)
    {
        LOG_ERROR("Failed to parse image, buffer broken. %s", path);
        return ret;
    }

    ret.p_buffer   = buffer;
    ret.buffer_len = length;

    return ret;
}