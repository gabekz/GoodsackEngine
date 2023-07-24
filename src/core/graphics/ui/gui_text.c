#include "gui_text.h"

#include <ctype.h>

GuiText *
gui_text_create(const char *text_string)
{
    GuiText *ret = malloc(sizeof(GuiText));

    ret->font_atlas =
      texture_create("../res/fonts/font.bmp",
                     NULL,
                     (TextureOptions) {1, GL_RGBA, false, true});

    ret->text = text_string;

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

    // Loop through each character
    for (int i = 0; i < char_count; i++) {
        char c = toupper(text_string[i]);

        int target = (int)c - atlas_first_char_ascii_num;
        if (target < 0) continue; // TODO

        int row = target % sprite_cells_rows;
        int col = (target / sprite_cells_cols);

        // invert y-axis
        col = sprite_cells_rows - 1 + (sprite_cell_y_incr * col);

        LOG_INFO("Char %c converts to [%d : %d]", c, row, col);

        // target is now the target *index* of the font atlas.

        // Texture coordinate conversion from sprite-sheet
        vec4 tex_coords = {0, 0, 0, 0};
        tex_coords[0] =
          ((row + 1) * sprite_cells_size[0]) / sprite_sheet_size[0];
        tex_coords[1] = ((row)*sprite_cells_size[0]) / sprite_sheet_size[0];

        tex_coords[2] =
          ((col + 1) * sprite_cells_size[1]) / sprite_sheet_size[1];
        tex_coords[3] = ((col)*sprite_cells_size[1]) / sprite_sheet_size[1];

        // CREATE NEW ELEMENTS
        ret->elements[i] = gui_element_create((vec2) {25 + (18 * i), 25},
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