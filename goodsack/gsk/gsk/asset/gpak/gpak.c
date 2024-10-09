/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gpak.h"

#include <stdio.h>
#include <stdlib.h>

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "asset/asset_cache.h"

gsk_GpakWriter
gsk_gpak_writer_init()
{
    gsk_GpakWriter ret;

    FILE *file;
    char *uri             = "gsk://test.bin";
    const char *full_path = GSK_PATH(uri);

    char *buff    = "GPAK";
    u32 buff_size = strlen(buff);

    file = fopen(full_path, "wb");
    if (!file) { LOG_CRITICAL("Failed to create file %s", full_path); }

    fwrite(buff, buff_size, 1, file);

    // TODO: write asset-type container block
    // -- contains start/end blocks

    ret.file_ptr = file;
    ret.is_ready = TRUE;

    return ret;
}

void
gsk_gpak_writer_populate_cache(gsk_GpakWriter *p_writer,
                               gsk_AssetCache *p_cache)
{
    // write the cache
    u8 err = (p_writer == NULL | p_writer->file_ptr == NULL) ? 1 : 0;
    if (err) { LOG_CRITICAL("Failed to open GPAK writer!"); }

    for (int i = 0; i < ASSETTYPE_LAST + 1; i++)
    {
        // TODO: update asset-type container block

        // go through each gcfg
        for (int j = 0; j < p_cache->asset_lists[i].list_state.list_next - 1;
             j++)
        {
            gsk_AssetCacheState *p_state;
            p_state = (gsk_AssetCacheState *)array_list_get_at_index(
              &(p_cache->asset_lists[i].list_state), j);

            char *uri;
            uri = (char *)array_list_get_at_index(&(p_cache->asset_uri_list),
                                                  p_state->asset_uri_index);

            // ASSET_HANDLE
            fwrite(&p_state->asset_handle, 1, sizeof(u64), p_writer->file_ptr);

#if 0
            // ASSET_URI_INDEX (maybe just put the uri here?)
            fwrite(
              &p_state->asset_uri_index, 1, sizeof(u32), p_writer->file_ptr);
#endif
            // ASSET URI
            fwrite(uri, strlen(uri), 1, p_writer->file_ptr);
            // START_BLOC_ID
            int p = 0xAAAAAAAA;
            fwrite(&p, 1, sizeof(u32), p_writer->file_ptr);
            // BLOCK_LEN
            int q = 0xBBBBBBBB;
            fwrite(&q, 1, sizeof(u32), p_writer->file_ptr);
        }
    }
}

void
gsk_gpak_writer_write(gsk_GpakWriter *p_writer)
{
    if (p_writer->file_ptr == NULL)
    {
        LOG_CRITICAL("Failed to open GPAK writer!");
    }
}
void
gsk_gpak_writer_close(gsk_GpakWriter *p_writer)
{
    fclose(p_writer->file_ptr);
}

#if 0
static void
gsk_gpak_write(void *buff, int buff_size)
{
    FILE *file;
    char *uri             = "gsk://test.bin";
    const char *full_path = GSK_PATH(uri);

    // reserve mem
    char *readed_buff = malloc(buff_size);

    file = fopen(full_path, "wb");
    if (!file) { LOG_CRITICAL("Failed to create file %s", full_path); }

    fwrite(buff, buff_size, 1, file);

    fclose(file);

    // now read info
    file = fopen(full_path, "rb");
    if (!file) { LOG_CRITICAL("Failed to read file!"); }
    fread(readed_buff, buff_size, 1, file);

    fclose(file);

    LOG_INFO("READ %s", readed_buff);
}

void
gsk_gpak_make_raw(gsk_AssetCache *p_cache)
{

    // get raw texture data
    // gsk_Texture tex = gsk_asset_cache_get_by_handle(p_cache, HANDLE);

    // 1. go through each texture asset
    // 2. get the handle and URI
    // 3. from the URI, get the image path
    // 4. get the raw image data
    // 5. store to gpak

    // need to store URI, then store binary location based on it

    // for (int i = 0; i < p_cache->asset_lists[0].list_state.list_next - 1;
    // i++)
    for (int i = 0; i < 1; i++)
    {
        gsk_AssetCacheState *p_state;

        p_state = (gsk_AssetCacheState *)array_list_get_at_index(
          &(p_cache->asset_lists[1].list_state), i);

        // grab the URI
        char *uri;
        uri = (char *)array_list_get_at_index(&(p_cache->asset_uri_list),
                                              p_state->asset_uri_index);

        const char *full_path = GSK_PATH(uri);

        // find the location on disk
        char *buffer = 0;
        long length;
        FILE *f = fopen(full_path, "rb");

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
            LOG_INFO("%c", buffer[0]);
            // start to process your data / extract strings here...
            gsk_gpak_write(buffer, length);

            free(buffer);
        }
    }
}
#endif