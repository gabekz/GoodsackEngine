#include "loader_obj.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/gfx.h>
#include <util/logger.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <core/graphics/mesh/mesh.h>

#define LOGGING_OBJ

#ifdef SYS_ENV_UNIX
#define strtok_s(x, y, z) strtok_r(x, y, z)
#endif

MeshData *
load_obj(const char *path, float scale)
{

    FILE *stream = NULL;
    char line[256]; // 256 = MAX line_length

    if ((stream = fopen(path, "rb")) == NULL) {
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

    // Looping through the file
    while (fgets(line, sizeof(line), stream)) {
        // Get the first two characters
        char def[5]; // TODO: Fix uninitialized;
        memcpy(def, line, 2);

        char delim[] = " ";
        char *str    = line;

        char *split = strtok(str, delim);  // line, split by spaces
        split       = strtok(NULL, delim); // split is now ignoring first value

        if (strstr(def, "v ") != NULL) {

            while (split != NULL) {
                float saved = atof(split) * 0.2f;
                v[vL]       = saved * scale;
                vL++;

                split = strtok(NULL, delim);
            }
        }
        if (strstr(def, "vt") != NULL) {
            while (split != NULL) {
                float saved = atof(split);
                vt[vtL]     = saved;
                vtL++;

                split = strtok(NULL, delim);
            }
        }
        if (strstr(def, "vn") != NULL) {
            while (split != NULL) {
                float saved = atof(split);
                vn[vnL]     = saved * scale;
                vnL++;

                split = strtok(NULL, delim);
            }
        }
        if (strstr(def, "f") != NULL) {
            char *collection = split;
            // Create the "collections" for each face
            while (collection != NULL) {

                // Go through each collection and grab the vertex
                char elemDem[] = "/";
                char *ptr      = collection;
                char *posn     = NULL;
                char *element  = strtok_s(ptr, elemDem, &posn);

                // Go through each element in the collection
                // Get the incremental steps for the components we need
                int inc = ((vL > 0) + (vtL > 0) + (vnL > 0));
                for (int j = 0; j < inc; j++) {
                    int saved = atoi(element);

                    // Vertex
                    if (j == 0 && vL > 0) {
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
                    }
                    // Texture
                    else if (j == 1 && vtL > 0) {
                        int loc = saved * 2 - 2;

                        out[outI]     = vt[loc];
                        out[outI + 1] = vt[loc + 1];
                        outI += 2;

                    }
                    // Normal
                    else if (j == 2 && vnL > 0) {
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

#if 0
    printf("Vertex values: \n");
    for(int i = 0; i < vL; i++) {
        printf("%f\n", v[i]);
    }
    printf("Texture values: \n");
    for(int i = 0; i < vtL; i++) {
        printf("%f\n", vt[i]);
    }
    printf("Normal values: \n");
    for(int i = 0; i < vnL; i++) {
        printf("%f\n", vn[i]);
    }
#endif

    // Output
    MeshData *ret    = malloc(sizeof(MeshData));
    ret->vertexCount = outI;

#if 1 // Calcuate TBN for each triangle/vertex
    ui32 totalTriangles = fL * 3;
    // float* outTBN = malloc(2 * 3 * totalTriangles * sizeof(float));
    float *outTBN    = malloc(totalTriangles * 3 * 2 * sizeof(GLfloat));
    ui32 cntTriangle = 0;
    for (int i = 0; i < totalTriangles; i += 3) {

        // if(i != 2684) continue;

        // ui32 inc = i + (3 * i);

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
        for (int k = 0; k < 3; k++) {
            tang[k]  = f * (del2[1] * edge1[k] - del1[1] * edge2[k]);
            btang[k] = f * (-del2[0] * edge1[k] + del1[0] * edge2[k]);
        }

        for (int m = 0; m < 3; m++) {
            int b         = i + m + (i * 5) + (m * 5);
            outTBN[b + 0] = tang[0];
            outTBN[b + 1] = tang[1];
            outTBN[b + 2] = tang[2];

            outTBN[b + 3] = btang[0];
            outTBN[b + 4] = btang[1];
            outTBN[b + 5] = btang[2];
        }

        // printf("\n%d", i);
    }
    // free(vboTBN);
#endif

#ifdef LOGGING_OBJ
    printf("-------------------------------------\n[OBJ Loader]\n");
    printf("path: \t\t%s", path);
    printf("\n\nVertice "
           "Count\nPosition:\t%d\nTexture:\t%d\nNormal:\t\t%d\n\nFaces:\t\t%"
           "d\nTris:\t\t%d\n\n",
           vL / 3,
           vtL / 2,
           vnL / 3,
           fL,
           totalTriangles);
    printf("Output Buffer Size: %.2f KB\n", (float)outI / 1000);
    printf("-------------------------------------\n\n");
#endif

    // ret->meshPath       = path;
    ret->trianglesCount = totalTriangles;

    ret->buffers.v  = v;
    ret->buffers.vt = vt;
    ret->buffers.vn = vn;

    ret->buffers.vL  = vL;
    ret->buffers.vtL = vtL;
    ret->buffers.vnL = vnL;

    ret->buffers.out = out;
    // TODO: fix this crap right here
    ret->buffers.outI   = outI * sizeof(float);
    ret->buffers.outTBN = outTBN;

    ret->buffers.bufferIndices_size = 0;

    ret->isSkinnedMesh = 0;
    ret->hasTBN        = 1;

    // glBindVertexArray(0);
    //  Free a lot of memory....
    // free(v);
    // free(vt);
    // free(vn);
    // free(out);
    // free(outIndices);
    // free(outIndices);
    //  .. and some more.
    // free(ibo);

    fclose(stream);

    return ret;
}
