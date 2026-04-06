/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gpak_archive.h"

#include "util/array_list.h"
#include "util/logger.h"

#include <string.h>

u8
gsk_archive_bytes(gsk_Archive *p_self, void *p_data, u32 data_size)
{
    if (p_self == NULL) { LOG_CRITICAL("Failed archive"); }

    if (p_self->mode == GskArchiveMode_Write)
    {
        LIST_APPEND(&p_self->out, p_data, data_size);
        return 1;
    }

    memcpy(p_data, (char *)p_self->p_buffer + p_self->seek_cnt, data_size);
    p_self->seek_cnt += data_size;

    return 1;
}

//#define _GOODSACK_FS_DIR_BUILD "D:/Projects/zweihunter/build/output/bin/Debug"