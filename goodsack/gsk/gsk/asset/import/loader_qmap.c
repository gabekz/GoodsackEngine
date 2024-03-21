/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_qmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#define QM_MODE_NONE     0
#define QM_MODE_FILL_ENT 1
#define QM_MODE_FILL_BSH 2

#define QM_M_SUB 0
#define QM_M_ADD 1

static int
__next_mode(int mode, int add)
{
    if ((mode >= QM_MODE_FILL_BSH && add == QM_M_ADD) ||
        (mode <= QM_MODE_NONE && add == QM_M_SUB)) {
        LOG_CRITICAL("Corrupt file. %d ", mode);
    }

    return (add) ? mode + 1 : mode - 1;
}

#define MODE_UP(x)   x = __next_mode(x, QM_M_ADD);
#define MODE_DOWN(x) x = __next_mode(x, QM_M_SUB);

static void
__read_plane(char *line)
{
    LOG_INFO("Reading point..");

    int cnt_char  = 0;                  // cnt reading character of the line
    int cnt_coord = 0, cnt_num = 0;     // cnt coords and numbers
    int start_index = 0, end_index = 0; // index for parenthesis-split

    // points
    vec3 points[3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    // Loop through line
    while (cnt_char != strlen(line)) {

        //
        // --- READ COORDINATES
        //

        // start of coordinate
        if (line[cnt_char] == '(') {
            start_index = cnt_char;
            cnt_num     = 0;
        }

        // end of coordinate
        if (line[cnt_char] == ')') {
            end_index = cnt_char + 1;

            // get coordiante substring
            char substring[256];
            strncpy(substring, line + (start_index), (end_index - start_index));
            substring[end_index - start_index] = '\0';

            start_index = cnt_char;
            end_index   = cnt_char;

            // LOG_INFO("%s", substring);

            // separate coordinate by spaces
            char delim[] = " ";
            char *str    = substring;

            char *split = strtok(str, delim); // line, split by spaces
            split = strtok(NULL, delim); // split is now ignoring first value

            // read each number in the coordinate
            while (split != NULL && cnt_num <= 2) {
                float saved = atof(split);
                split       = strtok(NULL, delim);

                // LOG_INFO("saved: %f", saved);

                points[cnt_coord][cnt_num] = saved;

                cnt_num++;
            }
            cnt_num = 0; // reset number
            cnt_coord++;
        }

        cnt_char++;
    }

    for (int i = 0; i < 3; i++) {
        LOG_INFO("x: %f y: %f z: %f", points[i][0], points[i][1], points[i][2]);
    }

    //
    // --- READ TEXTURE
    //

    //
    // --- READ TEXTURE COORDS
    //
}

gsk_QMapContainer
gsk_load_qmap(const char *map_path)
{
    FILE *stream = NULL;
    char line[256]; // 256 = MAX line_length

    if ((stream = fopen(map_path, "rb")) == NULL) {
        LOG_CRITICAL("Error opening %s\n", map_path);
        exit(1);
    }

    int current_mode = QM_MODE_NONE;

    while (fgets(line, sizeof(line), stream)) {
        switch (line[0]) {
        case '{': MODE_UP(current_mode); break;
        case '}': MODE_DOWN(current_mode); break;
        case '(': __read_plane(line); break;
        default: break;
        }
    }
}