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

#include "asset/asset.h"
#include "asset/asset_cache.h"
#include "asset/assetdefs.h"

static void
__rotate_data_file(gsk_GpakWriter *p_writer)
{
    if (p_writer->dat_file_count > 0) { fclose(p_writer->data_file_ptr); };

    FILE *dat_file;

    char headT[32];
    char pathT[256];

    sprintf(headT, "GPAK_PAGE%d", p_writer->dat_file_count);
    sprintf(pathT, "gsk://test_%d.gpak", p_writer->dat_file_count);

    const char *data_full_path = GSK_PATH(pathT);
    dat_file                   = fopen(data_full_path, "wb");
    if (!dat_file) { LOG_CRITICAL("Failed to create file %s", data_full_path); }

    fwrite(headT, strlen(headT), 1, dat_file);

    p_writer->data_file_ptr = dat_file;
    p_writer->dat_file_count += 1;
    p_writer->dat_file_crnt += 1;
}

gsk_GpakWriter
gsk_gpak_writer_init()
{
    gsk_GpakWriter ret = {0};

    FILE *file;
    char *uri             = "gsk://test.gpak";
    const char *full_path = GSK_PATH(uri);

    char *buff    = "GPAK";
    u32 buff_size = strlen(buff);

    file = fopen(full_path, "wb");
    if (!file) { LOG_CRITICAL("Failed to create file %s", full_path); }

    fwrite(buff, buff_size, 1, file);

    // TODO: write asset-type container block
    // -- contains start/end blocks

    ret.file_ptr      = file;
    ret.data_file_ptr = NULL;
    ret.is_ready      = TRUE;

    ret.dat_file_count = 0;
    ret.dat_file_crnt  = 0;

    __rotate_data_file(&ret);

    return ret;
}

void
gsk_gpak_writer_populate_cache(gsk_GpakWriter *p_writer,
                               gsk_AssetCache *p_cache)
{
    // write the cache
    u8 err = (p_writer == NULL || p_writer->file_ptr == NULL) ? 1 : 0;
    if (err) { LOG_CRITICAL("Failed to open GPAK writer!"); }

    for (int i = 0; i < ASSETTYPE_LAST + 1; i++)
    {
        // TODO: Temporarily only checking Textures
        if (i != GSK_ASSET_CACHE_TEXTURE) { continue; }

        // TODO: update asset-type container block

        for (int j = 0; j < p_cache->asset_lists[i].list_state.list_next - 1;
             j++)
        {
            /*---- capture AssetRef ------------------------------------------*/

            gsk_AssetRef *p_ref = NULL;
            p_ref               = (gsk_AssetRef *)array_list_get_at_index(
              &(p_cache->asset_lists[i].list_state), j);

            char *uri;
            uri = (char *)array_list_get_at_index(&(p_cache->asset_uri_list),
                                                  p_ref->asset_uri_index);

            p_ref =
              _gsk_asset_get_internal(p_cache, uri, GSK_ASSET_FETCH_IMPORT);

            if (p_ref == NULL) { LOG_ERROR("Failed to get asset"); }
            if (p_ref->is_imported == FALSE || p_ref->p_data_import == NULL)
            {
                LOG_ERROR("Failed to import asset");
            }

            /*---- BLOC info -------------------------------------------------*/

            u32 bloc_offset = 0;
            u32 bloc_length = 0;

            u8 bloc_pages[2] = {p_writer->dat_file_crnt, 0};

            /*==== Write Asset Blob Data =====================================*/

            gsk_AssetBlob asset_source = *(gsk_AssetBlob *)p_ref->p_data_import;

            // TODO: check page-spanning

            u32 size_check =
              ftell(p_writer->data_file_ptr) + asset_source.buffer_len;
            if (size_check >= GSK_GPAK_MAX_FILESIZE)
            {
                __rotate_data_file(p_writer);
            }

            bloc_offset = ftell(p_writer->data_file_ptr) + 1;
            bloc_length = asset_source.buffer_len;

            fwrite(asset_source.p_buffer,
                   asset_source.buffer_len,
                   1,
                   p_writer->data_file_ptr);

            // TODO: Move this somewhere else.
            // cleanup
            free(asset_source.p_buffer);

            // get bloc_page_end after writing asset blob
            bloc_pages[1] = p_writer->dat_file_crnt;

            /*==== Write Asset Info ==========================================*/

            // ASSET_HANDLE
            fwrite(&p_ref->asset_handle, 1, sizeof(u64), p_writer->file_ptr);

#if 0
            // ASSET_URI_INDEX
            fwrite(
              &p_ref->asset_uri_index, 1, sizeof(u32), p_writer->file_ptr);
#else
            // ASSET URI
            fwrite(uri, strlen(uri), 1, p_writer->file_ptr);
#endif
            // BLOC_PAGES
            fwrite(bloc_pages, 2, sizeof(u8), p_writer->file_ptr);
            // START_BLOC_ID -- offset
            fwrite(&bloc_offset, 1, sizeof(u32), p_writer->file_ptr);
            // BLOC_LEN
            fwrite(&bloc_length, 1, sizeof(u32), p_writer->file_ptr);
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