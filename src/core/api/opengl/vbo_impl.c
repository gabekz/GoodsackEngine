#include "glbuffer.h"

#include <stdlib.h>

VBO *
vbo_create(const void *data, unsigned int size)
{
    VBO *vbo      = malloc(sizeof(VBO));
    vbo->elements = calloc(0, sizeof(BufferElement));

    vbo->elementsSize = 0;
    vbo->stride       = 0;

    glGenBuffers(1, &vbo->id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->id);
    // DOCS: glBufferData: GL type of buffer, size, data, GL draw-type
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    return vbo;
}

void
vbo_bind(VBO *self)
{
    glBindBuffer(GL_ARRAY_BUFFER, self->id);
}

void
vbo_unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
vbo_destroy(VBO *self)
{
    glDeleteBuffers(1, &self->id);
}

void
vbo_push(VBO *self, GLuint count, GLuint type, GLuint normalized)
{

    unsigned int size = self->elementsSize + 1;

    void *p        = (void *)self->elements;
    self->elements = realloc(p, size * sizeof(BufferElement));

    BufferElement *element = self->elements + (size - 1);
    /*
    self->elements[newSize].count       = count;
    self->elements[newSize].type        = type;
    self->elements[newSize].normalized  = normalized;
    */
    element->count      = count;
    element->type       = type;
    element->normalized = normalized;

    self->stride += sizeof(getElementTypeSize(type)) * count;
    self->elementsSize = size;
}

int
getElementTypeSize(GLuint type)
{
    switch (type) {
    case GL_FLOAT: return sizeof(GLfloat);
    case GL_UNSIGNED_INT: return sizeof(GLuint);
    default: break;
    }
    return -1;
}
