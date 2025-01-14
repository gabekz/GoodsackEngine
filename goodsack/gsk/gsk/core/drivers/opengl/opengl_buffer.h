/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

/*-----------------------------------------
 * VAO, VBO and IBO header
 * --------------------------------------*/

#ifndef __OPENGL_BUFFER_H__
#define __OPENGL_BUFFER_H__

#include "util/gfx.h"

typedef struct gsk_GlBufferElement
{
    unsigned int count;
    unsigned int type;
    unsigned int normalized;
} gsk_GlBufferElement;

typedef struct gsk_GlVertexArray
{
    unsigned int id;
    gsk_GlBufferElement *elements;
    unsigned int elementsCount;
} gsk_GlVertexArray;

typedef struct gsk_GlVertexBuffer
{
    unsigned int id;
    unsigned int type;

    unsigned int stride;
    unsigned int elementsSize;
    gsk_GlBufferElement *elements;
} gsk_GlVertexBuffer;

typedef struct gsk_GlIndexBuffer
{
    unsigned int id;
    unsigned int count;
} gsk_GlIndexBuffer;

// gsk_GlVertexArray

gsk_GlVertexArray *
gsk_gl_vertex_array_create();

void
gsk_gl_vertex_array_bind(gsk_GlVertexArray *self);

void
gsk_gl_vertex_array_add_buffer(gsk_GlVertexArray *self,
                               gsk_GlVertexBuffer *vbo);

// VBO

gsk_GlVertexBuffer *
gsk_gl_vertex_buffer_create(const void *data, unsigned int size);

void
gsk_gl_vertex_buffer_bind(gsk_GlVertexBuffer *self);

void
gsk_gl_vertex_buffer_unbind();

void
gsk_gl_vertex_buffer_destroy(gsk_GlVertexBuffer *self);

void
gsk_gl_vertex_buffer_push(gsk_GlVertexBuffer *self,
                          GLuint count,
                          GLuint type,
                          GLuint normalized);

int
gsk_gl_get_element_type_size(GLuint type);

// IBO
gsk_GlIndexBuffer *
gsk_gl_index_buffer_create(const void *data, unsigned int size);

void
gsk_gl_index_buffer_bind(gsk_GlIndexBuffer *self);

void
gsk_gl_index_buffer_destroy(gsk_GlIndexBuffer *self);

#endif // __OPENGL_BUFFER_H__
