/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "opengl_buffer.h"

#include "util/sysdefs.h"

#include <stdlib.h>

gsk_GlVertexArray *
gsk_gl_vertex_array_create()
{
    gsk_GlVertexArray *ret = malloc(sizeof(gsk_GlVertexArray));
    ret->elementsCount     = 0;
    glGenVertexArrays(1, &ret->id);

    return ret;
}

void
gsk_gl_vertex_array_bind(gsk_GlVertexArray *self)
{
    glBindVertexArray(self->id);
}

void
gsk_gl_vertex_array_add_buffer(gsk_GlVertexArray *self, gsk_GlVertexBuffer *vbo)
{
    gsk_gl_vertex_array_bind(self);
    gsk_gl_vertex_buffer_bind(vbo);
    gsk_GlBufferElement *elements = vbo->elements;

    u32 offset      = 0;
    u32 newElements = 0;
    for (int i = 0; i < vbo->elementsSize; i++)
    {

        // The offset for any existing VBO's inside this VAO
        u32 j = i + self->elementsCount;

#ifdef LOGGING
        printf("\nvbo:elementsSize: %d\n", vbo->elementsSize);
        printf("vbo:stride: %d\n", vbo->stride);
        printf("element %d count: %d\n", j, elements[i].count);
#endif

        gsk_GlBufferElement element = elements[i];

        glEnableVertexAttribArray(j);
        glVertexAttribPointer(j,
                              element.count,
                              element.type,
                              element.normalized,
                              vbo->stride,
                              (const void *)offset);

        offset += element.count * gsk_gl_get_element_type_size(element.type);
        newElements++;
    }

    // Increment the element count
    self->elementsCount += newElements;
}

int
gsk_gl_get_element_type_size(GLuint type)
{
    switch (type)
    {
    case GL_FLOAT: return sizeof(GLfloat);
    case GL_UNSIGNED_INT: return sizeof(GLuint);
    default: break;
    }
    return -1;
}

gsk_GlVertexBuffer *
gsk_gl_vertex_buffer_create(const void *data, u32 size, GskOglUsageType usage)
{
    gsk_GlVertexBuffer *vbo = malloc(sizeof(gsk_GlVertexBuffer));
    vbo->elements           = calloc(0, sizeof(gsk_GlBufferElement));

    vbo->elementsSize = 0;
    vbo->stride       = 0;

    glGenBuffers(1, &vbo->id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->id);
    // DOCS: glBufferData: GL type of buffer, size, data, GL draw-type
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);

    return vbo;
}

void
gsk_gl_vertex_buffer_bind(gsk_GlVertexBuffer *self)
{
    glBindBuffer(GL_ARRAY_BUFFER, self->id);
}

void
gsk_gl_vertex_buffer_unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
gsk_gl_vertex_buffer_destroy(gsk_GlVertexBuffer *self)
{
    glDeleteBuffers(1, &self->id);
}

void
gsk_gl_vertex_buffer_push(gsk_GlVertexBuffer *self,
                          GLuint count,
                          GLuint type,
                          GLuint normalized)
{

    u32 size = self->elementsSize + 1;

    void *p        = (void *)self->elements;
    self->elements = realloc(p, size * sizeof(gsk_GlBufferElement));

    gsk_GlBufferElement *element = self->elements + (size - 1);
    /*
    self->elements[newSize].count       = count;
    self->elements[newSize].type        = type;
    self->elements[newSize].normalized  = normalized;
    */
    element->count      = count;
    element->type       = type;
    element->normalized = normalized;

    self->stride += sizeof(gsk_gl_get_element_type_size(type)) * count;
    self->elementsSize = size;
}

gsk_GlIndexBuffer *
gsk_gl_index_buffer_create(const void *data, u32 size, GskOglUsageType usage)
{
    gsk_GlIndexBuffer *ibo = malloc(sizeof(gsk_GlIndexBuffer));

    glGenBuffers(1, &ibo->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);

    return ibo;
}

void
gsk_gl_index_buffer_bind(gsk_GlIndexBuffer *self)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->id);
}

void
gsk_gl_index_buffer_destroy(gsk_GlIndexBuffer *self)
{
    glDeleteBuffers(1, &self->id);
}

void
ibo_unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
