#ifndef VBO_H
#define VBO_H

#include "gfx.h"
#include "stdlib.h"

typedef struct _VBO VBO;
typedef struct _BufferElement BufferElement;

struct _VBO
{
    unsigned int id;
    unsigned int type;

    unsigned int stride;
    BufferElement *elements; // components - i.e, positions, texCoords, etc.
    unsigned int elementsSize;
};

struct _BufferElement {
    unsigned int count;
    unsigned int type;
    unsigned int normalized;
};

VBO *vbo_create(const void* data, unsigned int size);

void vbo_bind(VBO *self);
void vbo_unbind();
void vbo_destroy(VBO *self);

void vbo_push(VBO *self, GLuint count, GLuint type, GLuint normalized);

int getElementTypeSize(GLuint type);

#endif
