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

gsk_GCFG
gsk_load_gcfg(const char *path)
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
        LOG_TRACE("key: %s", key);
        LOG_TRACE("value: %s", value);

        gsk_GCFGItem item = {0};
        item.key          = strdup(key);
        item.value        = strdup(value);
        array_list_push(&ret.list_items, &item);
    }

    fclose(stream);

    return ret;
}