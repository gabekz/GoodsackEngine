/*
 * Copyright (c) 2022-2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_obj.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/array_list.h"
#include "util/gfx.h"
#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/graphics/mesh/mesh.h"

#define LOGGING_OBJ

// TODO: May want to remove this - we should handle texture scaling directly
// from textures (or materials) in shader code
#define USING_TEX_SCALE 1

#ifdef SYS_ENV_UNIX
#define strtok_s(x, y, z) strtok_r(x, y, z)
#endif

#define _ARRAYLIST_CAP 100

gsk_MeshData *
gsk_load_obj(const char *path, float scale)
{

    FILE *stream = NULL;
    char line[256]; // 256 = MAX line_length

    if ((stream = fopen(path, "rb")) == NULL)
    {
        LOG_CRITICAL("Error opening %s\n", path);
        exit(1);
    }

#ifdef LOGGING_OBJ
    LOG_INFO("Loading OBJ at path: %s", path);
#endif

    ArrayList buff_pos  = array_list_init(sizeof(float), _ARRAYLIST_CAP);
    ArrayList buff_tex  = array_list_init(sizeof(float), _ARRAYLIST_CAP);
    ArrayList buff_norm = array_list_init(sizeof(float), _ARRAYLIST_CAP);

    ArrayList buff_out     = array_list_init(sizeof(float), _ARRAYLIST_CAP);
    ArrayList buff_out_tbn = array_list_init(sizeof(float), _ARRAYLIST_CAP);

    int n_faces = 0;

    // Mesh bounds
    vec3 minBounds = GLM_VEC3_ZERO_INIT;
    vec3 maxBounds = GLM_VEC3_ZERO_INIT;

    // Looping through the file
    while (fgets(line, sizeof(line), stream))
    {
        // Get the first two characters
        char def[5]; // TODO: Fix uninitialized;
        memcpy(def, line, 2);

        char delim[] = " ";
        char *str    = line;

        char *split = strtok(str, delim);  // line, split by spaces
        split       = strtok(NULL, delim); // split is now ignoring first value

        if (strstr(def, "v ") != NULL)
        {

            while (split != NULL)
            {
                float saved = atof(split) * scale;
                array_list_push(&buff_pos, &saved);
                split = strtok(NULL, delim);
            }
        }
        if (strstr(def, "vt") != NULL)
        {
            while (split != NULL)
            {
                float saved = atof(split) * ((USING_TEX_SCALE) ? scale : 1);
                array_list_push(&buff_tex, &saved);
                split = strtok(NULL, delim);
            }
        }
        if (strstr(def, "vn") != NULL)
        {
            while (split != NULL)
            {
                float saved = atof(split) * scale;
                array_list_push(&buff_norm, &saved);
                split = strtok(NULL, delim);
            }
        }
        if (strstr(def, "f") != NULL)
        {
            char *collection = split;
            // Create the "collections" for each face
            while (collection != NULL)
            {

                // Go through each collection and grab the vertex
                char elemDem[] = "/";
                char *ptr      = collection;
                char *posn     = NULL;
                char *element  = strtok_s(ptr, elemDem, &posn);

                // Go through each element in the collection
                // Get the incremental steps for the components we need
                int inc =
                  ((!buff_pos.is_list_empty) + (!buff_tex.is_list_empty) +
                   (!buff_norm.is_list_empty));

                for (int j = 0; j < inc; j++)
                {
                    int saved = atoi(element);

                    // Position
                    if (j == 0 && buff_pos.list_next > 0)
                    {
                        float *v = buff_pos.data.buffer;

                        int loc = saved * 3 - 3;
                        // Add vertex to positions
                        array_list_push(&buff_out, &v[loc]);
                        array_list_push(&buff_out, &v[loc + 1]);
                        array_list_push(&buff_out, &v[loc + 2]);

                        // TODO: Add to indices here

                        // Calculate Mesh Bounds
                        if (v[loc] < minBounds[0]) minBounds[0] = v[loc];
                        if (v[loc + 1] < minBounds[1])
                            minBounds[1] = v[loc + 1];
                        if (v[loc + 2] < minBounds[2])
                            minBounds[2] = v[loc + 2];

                        if (v[loc] > maxBounds[0]) maxBounds[0] = v[loc];
                        if (v[loc + 1] > maxBounds[1])
                            maxBounds[1] = v[loc + 1];
                        if (v[loc + 2] > maxBounds[2])
                            maxBounds[2] = v[loc + 2];
                    }
                    // Texture
                    else if (j == 1 && buff_tex.list_next > 0)
                    {
                        float *vt = buff_tex.data.buffer;
                        int loc   = saved * 2 - 2;

                        array_list_push(&buff_out, &vt[loc]);
                        array_list_push(&buff_out, &vt[loc + 1]);

                    }
                    // Normal
                    else if (j == 2 && buff_norm.list_next > 0)
                    {
                        float *vn = buff_norm.data.buffer;
                        int loc   = saved * 3 - 3;

                        array_list_push(&buff_out, &vn[loc]);
                        array_list_push(&buff_out, &vn[loc + 1]);
                        array_list_push(&buff_out, &vn[loc + 2]);
                    }

                    element = strtok_s(NULL, elemDem, &posn);
                }
                // Next element increment
                collection = strtok(NULL, " ");
            }
            // printf("\n");
            /*
            for(int i = 0; i < 3; i++) {
                element = strtok(NULL, colDem);
                printf("Collection: %s\n", element);
            }
            */

            n_faces++;
        }
    }

    // --
    // Close the file pointer
    fclose(stream);

    // Output
    gsk_MeshData *ret = malloc(sizeof(gsk_MeshData));

#if 1 // Calcuate TBN for each triangle/vertex
    u32 total_verts = n_faces * 3;

    u32 buffer_tbn_size = total_verts * 3 * 2 * sizeof(float);
    float *buffer_tbn   = malloc(buffer_tbn_size);

    float *out = buff_out.data.buffer;

    u32 cntTriangle = 0;

    for (int i = 0; i < total_verts; i += 3)
    {
        vec3 edge1 = GLM_VEC3_ZERO_INIT;
        vec3 edge2 = GLM_VEC3_ZERO_INIT;

        vec3 tang  = GLM_VEC3_ZERO_INIT;
        vec3 btang = GLM_VEC3_ZERO_INIT;

        vec2 del1 = GLM_VEC2_ZERO_INIT;
        vec2 del2 = GLM_VEC2_ZERO_INIT;

        int inc     = i + (7 * i);
        float *pos1 = (vec3) {out[inc], out[inc + 1], out[inc + 2]};
        float *pos2 = (vec3) {out[8 + inc], out[8 + inc + 1], out[8 + inc + 2]};
        float *pos3 =
          (vec3) {out[16 + inc + 1], out[16 + inc + 2], out[16 + inc + 3]};

        float *uv1 = (vec2) {out[3 + inc], out[3 + inc + 1]};
        float *uv2 = (vec2) {out[11 + inc], out[11 + inc + 1]};
        float *uv3 = (vec2) {out[19 + inc], out[19 + inc + 1]};

        // solve for edge
        glm_vec3_sub(pos2, pos1, edge1);
        glm_vec3_sub(pos3, pos1, edge2);

        // solve for delta
        glm_vec2_sub(uv2, uv1, del1);
        glm_vec2_sub(uv3, uv1, del2);

        float f = 1.0f / (del1[0] * del2[1] - del2[0] * del1[1]);

        // loop for coordinates - x=0, y=1, z=2
        for (int k = 0; k < 3; k++)
        {
            tang[k]  = f * (del2[1] * edge1[k] - del1[1] * edge2[k]);
            btang[k] = f * (-del2[0] * edge1[k] + del1[0] * edge2[k]);
        }

        for (int m = 0; m < 3; m++)
        {
            int b             = i + m + (i * 5) + (m * 5);
            buffer_tbn[b + 0] = tang[0];
            buffer_tbn[b + 1] = tang[1];
            buffer_tbn[b + 2] = tang[2];

            buffer_tbn[b + 3] = btang[0];
            buffer_tbn[b + 4] = btang[1];
            buffer_tbn[b + 5] = btang[2];
        }
    }
#endif // CalcTBN

    ret->trianglesCount = total_verts / 3;
    ret->vertexCount    = total_verts;

    ret->primitive_type = GskMeshPrimitiveType_Triangle;

    ret->mesh_buffers_count = 2;

    ret->mesh_buffers[0] = (gsk_MeshBuffer) {
      .p_buffer     = buff_out.data.buffer,
      .buffer_size  = buff_out.list_next * sizeof(float),
      .buffer_flags = (GskMeshBufferFlag_Positions |
                       GskMeshBufferFlag_Textures | GskMeshBufferFlag_Normals),
    };

    ret->mesh_buffers[1] = (gsk_MeshBuffer) {
      .p_buffer    = buffer_tbn,
      .buffer_size = buffer_tbn_size,
      .buffer_flags =
        (GskMeshBufferFlag_Tangents | GskMeshBufferFlag_Bitangents),
    };

    glm_vec3_copy(minBounds, ret->boundingBox[0]);
    glm_vec3_copy(maxBounds, ret->boundingBox[1]);

    free(buff_pos.data.buffer);
    free(buff_tex.data.buffer);
    free(buff_norm.data.buffer);

#ifdef LOGGING_OBJ
    f32 total_size = 0;
    LOG_PRINT("-------------------------------------\n[OBJ Loader]\n");
    LOG_PRINT("path: \t\t%s", path);

    LOG_PRINT("\n\nUnique Vertex Attribs\n"
              "position:\t%d\n"
              "texture:\t%d\n"
              "normal:\t\t%d\n",
              (buff_pos.list_next - 1) / 3,
              (buff_tex.list_next - 1) / 2,
              (buff_norm.list_next - 1) / 3);

    for (int i = 0; i < ret->mesh_buffers_count; i++)
    {
        total_size += ret->mesh_buffers[i].buffer_size;
        LOG_PRINT("Buffer %d\n"
                  "buffer_size:\t%d bytes\n"
                  "buffer_flags:\t%d\n",
                  i,
                  ret->mesh_buffers[i].buffer_size,
                  ret->mesh_buffers[i].buffer_flags);
    }

    LOG_PRINT("Output Total\n"
              "triangles:\t%d\n"
              "vertices:\t%d\n",
              ret->trianglesCount,
              ret->vertexCount);

    LOG_PRINT("\nOutput Buffer Size: %.2f KB\n", (float)total_size / 1000);

    LOG_PRINT("-------------------------------------\n\n");
#endif

    return ret;
}
