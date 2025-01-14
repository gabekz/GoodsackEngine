/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_gcfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/filesystem.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#if 0
gsk_parse_gcfg() // parse data

gsk_pack_gcfg() // gpak pack the data
gsk_unpack_gcfg() // gpak unpack the data

gsk_load_gcfg() // load, no matter what we use this func
-- will either run gsk_unpack() or gsk_parse()
#endif

static GskGCFGItemType
_get_value_type(const char *value)
{
    if (value == NULL || value == '\0') { return GskGCFGItemType_None; }

    char *endptr;

#if 0
    {
        long val = strtol(value, &endptr, 10);
        if (*endptr == '\0') { return GskGCFGItemType_Int; }
    }
#endif

    {
        double val = strtod(value, &endptr);
        if (*endptr == '\0') { return GskGCFGItemType_Number; }
    }

    return GskGCFGItemType_String;
}

static gsk_GCFG
_parse_gcfg(char *path)
{
    gsk_GCFG ret   = {0};
    ret.list_items = array_list_init(sizeof(gsk_GCFGItem), 6);

    FILE *stream = NULL;
    char line[256]; // 256 = MAX line_length

    if ((stream = fopen(path, "rb")) == NULL)
    {
        LOG_CRITICAL("Error opening %s", path);
    }
    while (fgets(line, sizeof(line), stream))
    {
        // Strip newline characters (\n or \r\n)
        size_t len = strlen(line);
        if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        {
            line[len - 1] = '\0';
            if (len > 1 && line[len - 2] == '\r') { line[len - 2] = '\0'; }
        }

        // separate coordinate by spaces
        char delim[] = " ";
        char *str    = line;

        char *key   = strtok(str, delim);  // line, split by spaces
        char *value = strtok(NULL, delim); // line, split by spaces

        gsk_GCFGItem item = {0};
        item.key          = strdup(key);
        item.value        = strdup(value);
        item.type         = _get_value_type(item.value);

        array_list_push(&ret.list_items, &item);
    }

    fclose(stream);

    return ret;
}

static gsk_GCFG
_upack_gcfg(const char *uri)
{
    // 1. get gpak handler
    // 2. get asset info from uri
    // 3. read blocks
    // 4. fill data; return
    gsk_GCFG ret = {0};
    return ret;
}

static void
_package_gcfg(gsk_GCFG *p_gcfg)
{
    // 1. get gpak handler
    // 2. add asset info to GPAK handler (asset cache stuff)
    // 3. add blob to GPAK handler

    // gsk_GPakWriter gsk_gpak_writer_init();
    // gsk_gpak_asset_write(handle, uri_idx, data, data_size);
}

gsk_GCFG
gsk_load_gcfg(const char *path)
{
    // if HOT
    return _parse_gcfg(path);

    // else
    // return gsk_unpack_gcfg();
}
