#include <loaders/loader_obj.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <model/mesh.h>

Model* load_obj(const char* path) {

    FILE* stream = NULL;
    char line[256]; // 256 = MAX line_length

    if((stream = fopen(path, "rb")) == NULL) {
        printf("Error opening %s\n", path);
        exit(1);
    }

    // TODO: scaling
    int vC  = 50000;
    int vtC = 50000;
    int vnC = 50000;

    // Buffers for storing input
    float *v  = malloc(vC * sizeof(float));
    float *vt = malloc(vtC * sizeof(float));
    float *vn = malloc(vnC * sizeof(float));

    int vL  = 0;
    int vtL = 0;
    int vnL = 0;
    int fL  = 0;

    // Buffers for output
    int outC = 50000;
    int outIndicesC = 50000;

    float *out = malloc(outC * sizeof(float));
    unsigned int *outIndices = malloc(outIndicesC * sizeof(unsigned int));

    int outI = 0;
    int outIndicesI = 0;

    // Looping through the file
    while(fgets(line, sizeof(line), stream)) {

        // Get the first two characters
        char def[5];                        // TODO: Fix uninitialized;
        memcpy(def, line, 2);

        char delim[] = " ";
        char *str = line;

        char *split = strtok(str, delim); // line, split by spaces
        split = strtok(NULL, delim); // split is now ignoring first value

        if(strstr(def, "v ") != NULL) {

            while(split != NULL) {
                float saved = atof(split) * 0.2f;
                v[vL] = saved;
                vL++;

                split = strtok(NULL, delim);
            }
        }
        if(strstr(def, "vt") != NULL) {
            while(split != NULL) {
                float saved = atof(split);
                vt[vtL] = saved;
                vtL++;

                split = strtok(NULL, delim);
            }
        }
        if(strstr(def, "vn") != NULL) {
            while(split != NULL) {
                float saved = atof(split);
                vn[vnL] = saved;
                vnL++;

                split = strtok(NULL, delim);
            }
        }
        if(strstr(def, "f") != NULL) {
            char *collection = split;
#ifdef LOGGING
            printf("\ncollection: %s", collection);
#endif
            // Create the "collections" for each face
            while(collection != NULL) {

                // Go through each collection and grab the vertex
                char elemDem[] = "/";
                char *ptr = collection;
                char *posn = NULL;
                char *element = strtok_r(ptr, elemDem, &posn);


                // Go through each element in the collection
                // TODO: change to 1 for only vertex
                for(int j = 0; j < 3; j++) {
                    int saved = atoi(element);
#ifdef LOGGING
                    printf(" [%d],", saved);
#endif

                    // Vertex
                    if(j == 0) {
                        int loc = saved*3-3;
// Add vertex to positions
                        out[outI] = v[loc];
                        out[outI+1] = v[loc+1];
                        out[outI+2] = v[loc+2];
                        outI += 3;

                        // Add to indices
                        unsigned int ind = saved; // FIX INDICES TO MATCH
                        outIndices[outIndicesI] = ind;
                        outIndicesI++;
                    }
                    // Texture
                    else if(j == 1) {
                        int loc = saved*2-2;

                        out[outI] = vt[loc];
                        out[outI+1] = vt[loc+1];
                        outI += 2;

                    }
                    // Normal
                    else if(j == 2) {
                        int loc = saved*3-3;

                        out[outI] = vn[loc];
                        out[outI+1] = vn[loc+1];
                        out[outI+2] = vn[loc+2];
                        outI += 3;
                    }

                    element = strtok_r(NULL, elemDem, &posn);
                }
                // Next element increment
                collection = strtok(NULL, " ");
            }
            //printf("\n");
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

    printf("\nVertex Points: \t%d\nUV Points: \t%d\nNormal Points: \t%d\nFaces Count: \t%d\n\n",
      vL/3, vtL/2, vnL/3, fL);

    // Create the VAO
    VAO* vao = vao_create();
    vao_bind(vao);

    printf("VLCount: %d", vL);

    VBO* vbo = vbo_create(out, outI * sizeof(float));
    //VBO* vbo = vbo_create(v, 24 * sizeof(float));
    //IBO* ibo = ibo_create(outIndices, (outIndicesI) * sizeof(unsigned int));

    // Push our data into our single VBO
    // NOTE: For only vertex we must disable the following two pushes
    vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);

    // VBO push -> VAO
    vao_add_buffer(vao, vbo);

#if 0
    printf("Indices [count: %d]:\n", outIndicesI);
    for(int i = 0; i < outIndicesI; i++) {
        printf("%d, ", outIndices[i]);
    }
    printf("\n");
#endif

    printf("\nsize of OUT: %d\n", outI);

    // Output
    Model *ret = malloc(sizeof(Model));
    ret->vao = vao;
    ret->vertexCount = outI;

    //glBindVertexArray(0);
    // Free a lot of memory....
    free(v);
    free(vn);
    free(out);
    free(outIndices);
    //free(outIndices);
    // .. and some more.
    free(vbo);
    //free(ibo);

    fclose(stream);

    return ret;
}
