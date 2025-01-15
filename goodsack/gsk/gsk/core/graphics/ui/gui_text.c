/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gui_text.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "util/filesystem.h"

static void
__fill_font_data(char *self_widths, const char *path_font_data)
{
    FILE *p_file;
    u32 map_width, map_height;
    u32 cell_width, cell_height;
    char start_character;

    p_file = fopen(path_font_data, "rb");
    if (p_file == NULL)
    {
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

    fclose(p_file);
}

gsk_GuiText *
gsk_gui_text_create(const char *text_string, vec2 pos_offset, vec3 color)
{
    gsk_GuiText *ret = malloc(sizeof(gsk_GuiText));

    const char *font_bmp_path = GSK_PATH("gsk://fonts/font1024.bmp");
    const char *font_dat_path = GSK_PATH("gsk://fonts/font1024-bfd.dat");

    LOG_DEBUG("Loading Font: \n%s\n%s", font_bmp_path, font_dat_path);

    ret->font_atlas = texture_create(
      font_bmp_path, NULL, (TextureOptions) {8, GL_RGBA, FALSE, TRUE});

    __fill_font_data(ret->char_spacing, font_dat_path);

    // Create elements for each character in the string
    u32 char_count       = strlen(text_string);
    ret->character_count = char_count;
    ret->elements        = malloc(sizeof(gsk_GuiElement *) * char_count);

    // sprite sheet info
    vec2 sprite_sheet_size = {1024.0f, 1024.0f};
    vec2 sprite_cells_size = {64.0f, 64.0f};

    // determine the correct sprite index...
    u32 sprite_cells_rows  = 16;
    u32 sprite_cells_cols  = 16;
    u32 sprite_cells_total = sprite_cells_rows * sprite_cells_cols;

    // sprite options
    f32 char_size       = 20.0f;
    vec2 start_pos      = GLM_VEC2_ZERO_INIT;
    vec3 text_color_rgb = GLM_VEC3_ZERO_INIT;

    // copy stuff
    glm_vec2_copy(pos_offset, start_pos);
    glm_vec3_copy(color, text_color_rgb);

    if (char_size <= 0) { LOG_CRITICAL("cannot have 0 character size."); }

    // set start_pos based on char_size
    start_pos[0] = start_pos[0] + (char_size / 2.0f);
    start_pos[1] = start_pos[1] + (char_size / 2.0f);

    // increments..
    u32 sprite_cell_x_incr = 1;  // to advance row, x +1
    u32 sprite_cell_y_incr = -1; // to advance col, y -1

    u32 atlas_first_char_ascii_num = 32;

    // u32 sprite_alphabet_index_begin =

    // store the last spacing (kerning)
    f32 last_effective_spacing = 0;

    f32 current_x_position =
      start_pos[0]; // Start position for the text (can be modified)

    // Loop through each character
    for (int i = 0; i < char_count; i++)
    {
        char c =
          (sprite_cells_rows > 8) ? text_string[i] : toupper(text_string[i]);

        int target = (int)c - atlas_first_char_ascii_num;
        if (target < 0) continue; // Skip invalid characters

        int row = target % sprite_cells_rows;
        int col = (target / sprite_cells_cols);

        // Invert y-axis
        col = sprite_cells_rows - 1 + (sprite_cell_y_incr * col);

        // Texture coordinate conversion from sprite-sheet
        vec4 tex_coords = {0, 0, 0, 0};
        tex_coords[0] =
          ((row + 1) * sprite_cells_size[0]) / sprite_sheet_size[0];
        tex_coords[1] = ((row)*sprite_cells_size[0]) / sprite_sheet_size[0];
        tex_coords[2] =
          ((col + 1) * sprite_cells_size[1]) / sprite_sheet_size[1];
        tex_coords[3] = ((col)*sprite_cells_size[1]) / sprite_sheet_size[1];

        // Kerning: Update the position based on character spacing
        f32 spacing =
          ret->char_spacing[target] / (sprite_cells_size[0] / char_size);

        // CREATE NEW ELEMENTS
        ret->elements[i] =
          gsk_gui_element_create((vec2) {current_x_position, start_pos[1]},
                                 (vec2) {char_size, char_size},
                                 text_color_rgb,
                                 ret->font_atlas,
                                 tex_coords);

        // Store the current spacing as the last effective one (for the next
        // loop iteration)
        last_effective_spacing = spacing;

        // Update current position by character width (include the base width of
        // the character)
        current_x_position += last_effective_spacing;
    }

    return ret;
}

void
gsk_gui_text_draw(gsk_GuiText *self, u32 shader_id)
{
    for (int i = 0; i < self->character_count; i++)
    {
        gsk_gui_element_draw(self->elements[i], shader_id);
    }
}