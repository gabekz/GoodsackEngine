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
#include "util/sysdefs.h"

typedef enum GskOglUsageType_ {
    GskOglUsageType_Static  = GL_STATIC_DRAW,
    GskOglUsageType_Dynamic = GL_DYNAMIC_DRAW,
    GskOglUsageType_Stream  = GL_STREAM_DRAW,
} GskOglUsageType_;

typedef s32 GskOglUsageType;

typedef struct gsk_GlBufferElement
{
    u32 count;
    u32 type;
    u32 normalized;
} gsk_GlBufferElement;

typedef struct gsk_GlVertexArray
{
    u32 id;
    gsk_GlBufferElement *elements;
    u32 elementsCount;
} gsk_GlVertexArray;

typedef struct gsk_GlVertexBuffer
{
    u32 id;
    u32 type;

    u32 stride;
    u32 elementsSize;
    gsk_GlBufferElement *elements;
} gsk_GlVertexBuffer;

typedef struct gsk_GlIndexBuffer
{
    u32 id;
    u32 count;
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
gsk_gl_vertex_buffer_create(const void *data, u32 size, GskOglUsageType usage);

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
gsk_gl_index_buffer_create(const void *data, u32 size, GskOglUsageType usage);

void
gsk_gl_index_buffer_bind(gsk_GlIndexBuffer *self);

void
gsk_gl_index_buffer_destroy(gsk_GlIndexBuffer *self);

#endif // __OPENGL_BUFFER_H__
