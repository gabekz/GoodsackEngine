#include "vao.h"

#include "stdio.h"
#include "stdlib.h"

#define LOGGING

VAO* vao_create() {
    VAO* ret = malloc(sizeof(VAO));
    ret->elementsCount = 0;
    glGenVertexArrays(1, &ret->id);

    return ret;
}

void vao_bind(VAO* self) {
    glBindVertexArray(self->id);
}

void vao_add_buffer(VAO* self, VBO* vbo) {
    vao_bind(self);
    vbo_bind(vbo);
    BufferElement *elements = vbo->elements;

    unsigned int offset = 0;
    unsigned int newElements = 0;
    for(int i = 0; i < vbo->elementsSize; i++) {

        // The offset for any existing VBO's inside this VAO
        unsigned int j = i + self->elementsCount;

#ifdef LOGGING
        printf("\nvbo:elementsSize: %d\n", vbo->elementsSize);
        printf("vbo:stride: %d\n", vbo->stride);
        printf("element %d count: %d\n", j, elements[i].count);
#endif

        BufferElement element = elements[i];

        glEnableVertexAttribArray(j);
        glVertexAttribPointer(j, element.count, element.type,
          element.normalized, vbo->stride, (const void*)offset);

        offset += element.count * getElementTypeSize(element.type);
        newElements++;
    }

    // Increment the element count
    self->elementsCount += newElements;

}
