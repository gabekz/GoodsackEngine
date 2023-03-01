#include "opengl_buffer.h"
#include <stdlib.h>

VAO *
vao_create()
{
    VAO *ret           = malloc(sizeof(VAO));
    ret->elementsCount = 0;
    glGenVertexArrays(1, &ret->id);

    return ret;
}

void
vao_bind(VAO *self)
{
    glBindVertexArray(self->id);
}

void
vao_add_buffer(VAO *self, VBO *vbo)
{
    vao_bind(self);
    vbo_bind(vbo);
    BufferElement *elements = vbo->elements;

    unsigned int offset      = 0;
    unsigned int newElements = 0;
    for (int i = 0; i < vbo->elementsSize; i++) {

        // The offset for any existing VBO's inside this VAO
        unsigned int j = i + self->elementsCount;

#ifdef LOGGING
        printf("\nvbo:elementsSize: %d\n", vbo->elementsSize);
        printf("vbo:stride: %d\n", vbo->stride);
        printf("element %d count: %d\n", j, elements[i].count);
#endif

        BufferElement element = elements[i];

        glEnableVertexAttribArray(j);
        glVertexAttribPointer(j,
                              element.count,
                              element.type,
                              element.normalized,
                              vbo->stride,
                              (const void *)offset);

        offset += element.count * getElementTypeSize(element.type);
        newElements++;
    }

    // Increment the element count
    self->elementsCount += newElements;
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
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);

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

IBO *
ibo_create(void *data, unsigned int size)
{
    IBO *ibo = malloc(sizeof(IBO));

    glGenBuffers(1, &ibo->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);

    return ibo;
}

void
ibo_bind(IBO *self)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->id);
}

void
ibo_destroy(IBO *self)
{
    glDeleteBuffers(1, &self->id);
}

void
ibo_unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
