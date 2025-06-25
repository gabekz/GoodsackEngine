/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gpak.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gsk_generated/GoodsackEngineConfig.h"

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

    if (p_writer->p_cache == NULL)
    {
        LOG_CRITICAL("Failed to rotate data file - asset cache is NULL");
    }

    FILE *dat_file;

    char headT[32];
    char pathT[256];

    sprintf(headT, "GPAK_PAGE%d", p_writer->dat_file_count);
    sprintf(pathT,
            "%s%s_%d.gpak",
            p_writer->output_dir,
            p_writer->p_cache->cache_scheme,
            p_writer->dat_file_count);

    // const char *data_full_path = GSK_PATH(pathT);
    dat_file = fopen(pathT, "wb");
    if (!dat_file) { LOG_CRITICAL("Failed to create file %s", pathT); }

    fwrite(headT, strlen(headT), 1, dat_file);

    p_writer->data_file_ptr = dat_file;
    p_writer->dat_file_count += 1;
    p_writer->dat_file_crnt += 1;
}

gsk_GpakWriter
gsk_gpak_writer_init(gsk_AssetCache *p_cache, const char *output_absolute_dir)
{
    gsk_GpakWriter ret = {0};

    if (p_cache == NULL)
    {
        LOG_CRITICAL("Failed to start writer - asset cache is NULL");
    }
    ret.p_cache = p_cache;
    strcpy(ret.output_dir, output_absolute_dir);

    FILE *file;

    char file_dest[GSK_FS_MAX_PATH];
    sprintf(file_dest, "%s%s.gpak", ret.output_dir, p_cache->cache_scheme);

    char head[6] = "GPAK_";

    char buff_head[128];
    sprintf(buff_head, "%s", head);

    file = fopen(file_dest, "wb");
    if (!file) { LOG_CRITICAL("Failed to create file %s", file_dest); }

    fwrite(buff_head, strlen(buff_head), 1, file);

    u32 n_assets_fill = 0;
    fwrite(&n_assets_fill, 1, sizeof(u32), file);

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
gsk_gpak_writer_populate_cache(gsk_GpakWriter *p_writer)
{
    // write the cache
    u8 err = (p_writer == NULL || p_writer->file_ptr == NULL) ? 1 : 0;
    if (err) { LOG_CRITICAL("Failed to open GPAK writer!"); }

    gsk_AssetCache *p_cache = p_writer->p_cache;
    if (p_cache == NULL)
    {
        LOG_CRITICAL("Failed to populate cache in writer. Lost p_cache");
    }

    u32 total_assets = 0;

    for (int i = 0; i < ASSETTYPE_LAST + 1; i++)
    {
        // TODO: Temporarily only checking Textures
        if (i != GskAssetType_Texture && i != GskAssetType_Model) { continue; }

        // TODO: update asset-type container block

        int total_refs = p_cache->asset_lists[i].list_state.list_next - 1;
        for (int j = 0; j < total_refs; j++)
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

            if (p_ref == NULL)
            {
                LOG_ERROR("Failed to get asset");
                continue;
            }
            if (p_ref->is_imported == FALSE || p_ref->p_data_import == NULL)
            {
                LOG_ERROR("Failed to import asset: %s", uri);
                continue;
            }

            /*---- BLOC info -------------------------------------------------*/

            u32 bloc_offset = 0;
            u32 bloc_length = 0;

            u8 bloc_pages[2] = {p_writer->dat_file_crnt, 0};

            /*==== Write Asset Blob Data =====================================*/

            gsk_AssetBlob asset_source = *(gsk_AssetBlob *)p_ref->p_data_import;

            if (asset_source.is_serialized == FALSE)
            {
                LOG_ERROR("Asset not serialized. Failed to write asset: %s",
                          uri);
                continue;
            }

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

            // BLOC_PAGES
            fwrite(bloc_pages, 2, sizeof(u8), p_writer->file_ptr);
            // START_BLOC_ID -- offset
            fwrite(&bloc_offset, 1, sizeof(u32), p_writer->file_ptr);
            // BLOC_LEN
            fwrite(&bloc_length, 1, sizeof(u32), p_writer->file_ptr);

            // ASSET URI

            gsk_URI suri = gsk_filesystem_uri(uri);
            s32 suri_len = strlen(suri.path);

            LOG_DEBUG("writing asset: %s", uri);

            fwrite(&suri_len, 1, sizeof(s32), p_writer->file_ptr);
            fwrite(suri.path, suri_len, 1, p_writer->file_ptr);

            total_assets += 1;
        }
    }

    fseek(p_writer->file_ptr, 5, SEEK_SET);
    fwrite(&total_assets, 1, sizeof(u32), p_writer->file_ptr);
}

void
gsk_gpak_writer_close(gsk_GpakWriter *p_writer)
{
    fclose(p_writer->file_ptr);
}

#pragma pack(push, 1)
struct _BlocInfo
{
    u64 handle;
    u8 bloc_pages[2];
    u32 bloc_offset;
    u32 bloc_length;
    u32 path_len;
};
#pragma pack(pop)

void
gsk_gpak_reader_fill_cache(gsk_AssetCache *p_cache, const char *gpak_path)
{
    // TODO: find dict file based on AssetCache in gpak_directory

    FILE *file_dic;
    char magic[5];

    file_dic = fopen(gpak_path, "rb");
    if (!file_dic)
    {
        LOG_CRITICAL("Failed to open dictionary file %s", gpak_path);
    }

    magic[4] = '\0';
    fread(magic, strlen("GPAK"), 1, file_dic);

    if (strcmp(magic, "GPAK"))
    {
        LOG_ERROR("First 4 bytes should be \"GPAK\", are \"%4s\"", magic);
    }
    // TODO: get scheme instead of seeking 1 byte
    if (fseek(file_dic, 1, SEEK_CUR) != 0) { LOG_CRITICAL("fseek failed"); }

    u32 n_assets = 0;
    fread(&n_assets, 1, sizeof(u32), file_dic);

    for (int i = 0; i < n_assets; i++)
    {
        struct _BlocInfo bloc_read = {0};

        if (fread(&bloc_read, sizeof(bloc_read), 1, file_dic) != 1)
        {
            LOG_CRITICAL("Failed to read asset info");
        }

        if (bloc_read.path_len > GSK_FS_MAX_PATH)
        {
            LOG_CRITICAL("URI length is too large.");
        }

        char path[GSK_FS_MAX_PATH] = "";
        if (fread(path, bloc_read.path_len, 1, file_dic) != 1)
        {
            LOG_CRITICAL("Failed to read asset URI from info");
        }

        char uri[GSK_FS_MAX_PATH] = "";
        sprintf(uri, "%s://%s", p_cache->cache_scheme, path);

        LOG_DEBUG(
          "%d, %d, %s", bloc_read.bloc_offset, bloc_read.bloc_length, uri);

        // validate URI
        {
            gsk_URI uri_check = gsk_filesystem_uri(uri);
            if (uri_check.path == '\0')
            {
                LOG_CRITICAL("Failed to parse asset URI from bloc. Corrupted.");
            }
        }

        // create BLOC info to pass to cache
        gsk_AssetBlocInfo bloc = {
          .bloc_length   = bloc_read.bloc_length,
          .bloc_offset   = bloc_read.bloc_offset,
          .bloc_pages[0] = bloc_read.bloc_pages[0],
          .bloc_pages[1] = bloc_read.bloc_pages[1],
        };

        u32 list_type = GSK_ASSET_HANDLE_LIST_NUM(bloc_read.handle);

        // send to asset cache
        gsk_asset_cache_add(p_cache, list_type, uri, &bloc);
    }

    fclose(file_dic);
}

gsk_AssetBlob
gsk_gpak_reader_import_blob(const char *uri_str)
{
    // TODO: Spanning

    gsk_AssetCache *p_cache = gsk_runtime_get_asset_cache(uri_str);
    gsk_AssetRef *p_ref     = gsk_asset_cache_get(p_cache, uri_str);
    const char *path        = (_GOODSACK_FS_DIR_BUILD "/output/gpak/");

    char pathT[256];
    sprintf(pathT,
            "%s%s_%d.gpak",
            path,
            p_cache->cache_scheme,
            p_ref->bloc_info.bloc_pages[0] - 1);

    // check with FS to open
    FILE *file_ptr;
    file_ptr = fopen(pathT, "rb");
    if (!file_ptr) { LOG_CRITICAL("Failed to open file: %s", pathT); }

    char *buffer    = NULL;
    long buffer_len = p_ref->bloc_info.bloc_length;

    buffer = malloc(buffer_len);
    if (buffer == NULL) { LOG_CRITICAL("Failed to allocate AssetBlob buffer"); }

    fseek(file_ptr, p_ref->bloc_info.bloc_offset - 1, SEEK_SET);
#if 0
    if (fread(buffer, 1, buffer_len, file_ptr) != 1)
    {
        LOG_CRITICAL("Failed to read buffer");
    }
#else
    fread(buffer, 1, buffer_len, file_ptr);
#endif

    gsk_AssetBlob ret = {
      .p_buffer   = buffer,
      .buffer_len = buffer_len,
    };

    // TODO: close - Need to handle this somewhere. Probably in the runtime.
    fclose(file_ptr);

    return ret;
}