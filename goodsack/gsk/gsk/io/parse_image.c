#include "parse_image.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "asset/assetdefs.h"

gsk_AssetBlob
parse_image(const char *path)
{
    gsk_AssetBlob ret = {0};

    // find the location on disk
    char *buffer = 0;
    long length;
    FILE *f = fopen(path, "rb");

    if (f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length);
        if (buffer) { fread(buffer, 1, length, f); }
        fclose(f);
    }

    if (buffer)
    {
        ret.p_buffer   = buffer;
        ret.buffer_len = length;
        // free(buffer);
    }

    return ret;
}