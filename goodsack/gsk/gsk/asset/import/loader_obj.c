/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_obj.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    // TODO: scaling
    int vC  = 1000000;
    int vtC = 1000000;
    int vnC = 1000000;

    // Buffers for storing input
    float *v  = malloc(vC * sizeof(float));
    float *vt = malloc(vtC * sizeof(float));
    float *vn = malloc(vnC * sizeof(float));

    int vL  = 0;
    int vtL = 0;
    int vnL = 0;
    int fL  = 0;

    // Buffers for output
    int outC        = 1000000;
    int outIndicesC = 1000000;

    float *out               = malloc(outC * sizeof(float));
    unsigned int *outIndices = malloc(outIndicesC * sizeof(unsigned int));

    int outI        = 0;
    int outIndicesI = 0;

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
                float saved = atof(split);
                v[vL]       = saved * scale;
                vL++;

                split = strtok(NULL, delim);
            }
        }
        if (strstr(def, "vt") != NULL)
        {
            while (split != NULL)
            {
                float saved = atof(split);
#if USING_TEX_SCALE
                vt[vtL] = saved * scale;
#else
                vt[vtL] = saved;
#endif // USING_TEX_SCALE

                vtL++;

                split = strtok(NULL, delim);
            }
        }
        if (strstr(def, "vn") != NULL)
        {
            while (split != NULL)
            {
                float saved = atof(split);
                vn[vnL]     = saved * scale;
                vnL++;

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
                int inc = ((vL > 0) + (vtL > 0) + (vnL > 0));
                for (int j = 0; j < inc; j++)
                {
                    int saved = atoi(element);

                    // Vertex
                    if (j == 0 && vL > 0)
                    {
                        int loc = saved * 3 - 3;
                        // Add vertex to positions
                        out[outI]     = v[loc];
                        out[outI + 1] = v[loc + 1];
                        out[outI + 2] = v[loc + 2];
                        outI += 3;

                        // Add to indices
                        unsigned int ind        = saved; // FIX INDICES TO MATCH
                        outIndices[outIndicesI] = ind;
                        outIndicesI++;

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
                    else if (j == 1 && vtL > 0)
                    {
                        int loc = saved * 2 - 2;

                        out[outI]     = vt[loc];
                        out[outI + 1] = vt[loc + 1];
                        outI += 2;

                    }
                    // Normal
                    else if (j == 2 && vnL > 0)
                    {
                        int loc = saved * 3 - 3;

                        out[outI]     = vn[loc];
                        out[outI + 1] = vn[loc + 1];
                        out[outI + 2] = vn[loc + 2];
                        outI += 3;
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

            fL++;
        }
    }

    // --
    // Close the file pointer
    fclose(stream);

    // Output
    gsk_MeshData *ret = malloc(sizeof(gsk_MeshData));
    ret->vertexCount  = outI;

#if 1 // Calcuate TBN for each triangle/vertex
    u32 total_verts = fL * 3;
    // float* buffer_tbn = malloc(2 * 3 * total_verts * sizeof(float));
    float *buffer_tbn = malloc(total_verts * 3 * 2 * sizeof(GLfloat));
    u32 cntTriangle   = 0;
    for (int i = 0; i < total_verts; i += 3)
    {

        // if(i != 2684) continue;

        // u32 inc = i + (3 * i);

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
        // printf("pos1: %f,%f,%f\n", pos1[0], pos1[1], pos1[2]);
        // printf("pos2: %f,%f,%f\n", pos2[0], pos2[1], pos2[2]);
        // printf("pos3: %f,%f,%f\n", pos3[0], pos3[1], pos3[2]);

        float *uv1 = (vec2) {out[3 + inc], out[3 + inc + 1]};
        float *uv2 = (vec2) {out[11 + inc], out[11 + inc + 1]};
        float *uv3 = (vec2) {out[19 + inc], out[19 + inc + 1]};
        // printf("\nvt1 %f %f ", uv1[0], uv1[1]);
        // printf("\nvt2 %f %f ", uv2[0], uv2[1]);
        // printf("\nvt3 %f %f ", uv3[0], uv3[1]);

        // solve for edge
        glm_vec3_sub(pos2, pos1, edge1);
        glm_vec3_sub(pos3, pos1, edge2);

        // solve for delta
        glm_vec2_sub(uv2, uv1, del1);
        glm_vec2_sub(uv3, uv1, del2);

        float f = 1.0f / (del1[0] * del2[1] - del2[0] * del1[1]);
        // float f = (isinf(d) || isnan(d)) ? 1.0f : d;
        // printf("\nd1 %f %f ", del1[0], del1[1]);
        // printf("\nd2 %f %f ", del2[0], del2[1]);
        // printf("\nF: %f", f);

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
#endif

#ifdef LOGGING_OBJ
    LOG_PRINT("-------------------------------------\n[OBJ Loader]\n");
    LOG_PRINT("path: \t\t%s", path);
    LOG_PRINT("\n\nVertice "
              "Count\nPosition:\t%d\nTexture:\t%d\nNormal:\t\t%d\n",
              vL / 3,
              vtL / 2,
              vnL / 3);
    LOG_PRINT("Output Total\n"
              "Vertices:\t%d\nTriangles:\t%d\n\n",
              total_verts,
              fL);
    LOG_PRINT("Output Buffer Size: %.2f KB\n", (float)outI / 1000);
    LOG_PRINT("-------------------------------------\n\n");
#endif

    ret->trianglesCount = total_verts;
    ret->hasTBN         = 0;
    ret->isSkinnedMesh  = 0;
    ret->has_indices    = FALSE;
    ret->primitive_type = GskMeshPrimitiveType_Triangle;

    ret->mesh_buffers_count = 2;

    ret->mesh_buffers[0] = (gsk_MeshBuffer) {
      .p_buffer     = out,
      .buffer_size  = outI * sizeof(float),
      .buffer_flags = (GskMeshBufferFlag_Positions |
                       GskMeshBufferFlag_Textures | GskMeshBufferFlag_Normals),
    };

    ret->mesh_buffers[1] = (gsk_MeshBuffer) {
      .p_buffer    = buffer_tbn,
      .buffer_size = total_verts * 3 * 2 * sizeof(float),
      .buffer_flags =
        (GskMeshBufferFlag_Tangents | GskMeshBufferFlag_Bitangents),
    };

    glm_vec3_copy(minBounds, ret->boundingBox[0]);
    glm_vec3_copy(maxBounds, ret->boundingBox[1]);

    free(v);
    free(vt);
    free(vn);

    return ret;
}
