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
    gsk_GCFG ret = {0};

    FILE *stream = NULL;
    char line[256]; // 256 = MAX line_length

    if ((stream = fopen(path, "rb")) == NULL)
    {
        LOG_CRITICAL("Error opening %s", path);
    }
    while (fgets(line, sizeof(line), stream))
    {
        // separate coordinate by spaces
        char delim[] = " ";
        char *str    = line;

        char *key   = strtok(str, delim);  // line, split by spaces
        char *value = strtok(NULL, delim); // line, split by spaces
        LOG_TRACE("key: %s", key);
        LOG_TRACE("value: %s", value);
    }

    fclose(stream);

    return ret;
}