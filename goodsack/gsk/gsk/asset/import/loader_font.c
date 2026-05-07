/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_font.h"

#include "io/parse_image.h"
#include "util/filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u8
__fill_font_data(char *self_widths, const char *path_font_data)
{
    FILE *p_file;
    u32 map_width, map_height;
    u32 cell_width, cell_height;
    char start_character;

    p_file = fopen(path_font_data, "rb");

    if (p_file == NULL)
    {
        LOG_ERROR("Failed to openl file: %s", path_font_data);
        return 0;
    }

    fread(&map_width, 4, 1, p_file);
    fread(&map_height, 4, 1, p_file);
    fread(&cell_width, 4, 1, p_file);
    fread(&cell_height, 4, 1, p_file);

    int total_chars = (map_width / cell_width) * (map_height / cell_height);

    fread(&start_character, 1, 1, p_file);
    // LOG_INFO("starting char is: %c", start_character);

    // TODO: Not sure why we have 32 bullshit bytes in the .dat file
    char filler[32];
    fread(&filler, 1, 32, p_file);

    fread(self_widths, 1, total_chars, p_file);
#if 0
    for (int i = 0; i < total_chars; i++) {
        LOG_INFO("%d", self_widths[i]);
    }
#endif

    fclose(p_file);
    return 1;
}

gsk_Font
gsk_font_import_from_file(const char *path)
{
    gsk_Font ret = {0};

    // ret.p_texture =
    //  texture_create(path, NULL, (TextureOptions) {8, GL_RGB, FALSE, TRUE});

    ret.image_blob = parse_image(path);

    const char *filename = strdup(path);
    sprintf(filename, "%s", gsk_filesystem_get_filename(filename));
    gsk_filesystem_strip_extension(filename);

    char path_dat[GSK_FS_MAX_PATH] = "";
    strncpy(path_dat, path, strlen(path));
    gsk_filesystem_strip_filename(path_dat);

    sprintf(path_dat, "%s/%s.dat", path_dat, filename);

    __fill_font_data(ret.char_spacing, path_dat);

    free(filename);

    return ret;
}