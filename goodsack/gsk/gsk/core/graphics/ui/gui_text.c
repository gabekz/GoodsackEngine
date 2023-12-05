/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gui_text.h"

#include <ctype.h>
#include <stdio.h>

#include "util/filesystem.h"

static void
__fill_font_data(char *self_widths, const char *path_font_data)
{
    FILE *p_file;
    ui32 map_width, map_height;
    ui32 cell_width, cell_height;
    char start_character;

    p_file = fopen(path_font_data, "rb");
    if (p_file == NULL) {
        LOG_CRITICAL("Failed to openl file: %s", path_font_data);
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
}

GuiText *
gui_text_create(const char *text_string)
{
    GuiText *ret = malloc(sizeof(GuiText));

    const char *font_bmp_path = GSK_PATH("gsk://fonts/font.bmp");
    const char *font_dat_path = GSK_PATH("gsk://fonts/font-bfd.dat");

    LOG_DEBUG("Loading Font: \n%s\n%s", font_bmp_path, font_dat_path);

    ret->font_atlas = texture_create(
      font_bmp_path, NULL, (TextureOptions) {1, GL_RGBA, false, true});

    __fill_font_data(ret->char_spacing, font_dat_path);

    // Create elements for each character in the string
    ui32 char_count      = strlen(text_string);
    ret->character_count = char_count;
    ret->elements        = malloc(sizeof(GuiElement *) * char_count);

    // sprite sheet info
    vec2 sprite_sheet_size = {256.0f, 256.0f};
    vec2 sprite_cells_size = {32.0f, 32.0f};

    // determine the correct sprite index...
    ui32 sprite_cells_rows  = 8;
    ui32 sprite_cells_cols  = 8;
    ui32 sprite_cells_total = sprite_cells_rows * sprite_cells_cols;

    // increments..
    ui32 sprite_cell_x_incr = 1;  // to advance row, x +1
    ui32 sprite_cell_y_incr = -1; // to advance col, y -1

    ui32 atlas_first_char_ascii_num = 32;

    // ui32 sprite_alphabet_index_begin =

    // store the last spacing (kerning)
    int last_effective_spacing = 10;

    // Loop through each character
    for (int i = 0; i < char_count; i++) {
        char c = toupper(text_string[i]);

        int target = (int)c - atlas_first_char_ascii_num;
        if (target < 0) continue; // TODO

        // NOTE: Target index references the character effective width
        // LOG_INFO("Target: %d, spacing: %d", target,
        // ret->char_spacing[target]);

        int row = target % sprite_cells_rows;
        int col = (target / sprite_cells_cols);

        // invert y-axis
        col = sprite_cells_rows - 1 + (sprite_cell_y_incr * col);

        // LOG_INFO("Char %c converts to [%d : %d]", c, row, col);

        // target is now the target *index* of the font atlas.

        // Texture coordinate conversion from sprite-sheet
        vec4 tex_coords = {0, 0, 0, 0};
        tex_coords[0] =
          ((row + 1) * sprite_cells_size[0]) / sprite_sheet_size[0];
        tex_coords[1] = ((row)*sprite_cells_size[0]) / sprite_sheet_size[0];

        tex_coords[2] =
          ((col + 1) * sprite_cells_size[1]) / sprite_sheet_size[1];
        tex_coords[3] = ((col)*sprite_cells_size[1]) / sprite_sheet_size[1];

        // Spacing
        int base_spacing = 25;
        int spacing      = (base_spacing * i); // + last_effective_spacing;

        // store Kerning
        last_effective_spacing = (int)(ret->char_spacing[target]);
        // LOG_INFO("new last effective: %d", last_effective_spacing);

        // CREATE NEW ELEMENTS
        ret->elements[i] = gui_element_create(
          (vec2) {50 + (16.5f * i) - last_effective_spacing, 25},
          (vec2) {50, 50},
          ret->font_atlas,
          tex_coords);
    }

    return ret;
}

void
gui_text_draw(GuiText *self)
{
    for (int i = 0; i < self->character_count; i++) {
        gui_element_draw(self->elements[i]);
    }
}