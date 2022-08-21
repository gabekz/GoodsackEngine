/*-----------------------------------------
 * VAO, VBO and IBO header
 * --------------------------------------*/

#ifndef GLBUFFER_H
#define GLBUFFER_H

#include "gfx.h"

typedef struct _VAO VAO;
typedef struct _VBO VBO;
typedef struct _IBO IBO;

typedef struct _BufferElement BufferElement;

struct _VAO {
   unsigned int id;
   BufferElement *elements;
   unsigned int elementsCount;
};

struct _VBO {
    unsigned int id;
    unsigned int type;

    unsigned int stride;
    BufferElement *elements; // components - i.e, positions, texCoords, etc.
    unsigned int elementsSize;
};

struct _IBO{
   unsigned int id;
   unsigned int count;
};

struct _BufferElement {
    unsigned int count;
    unsigned int type;
    unsigned int normalized;
};

// VAO
VAO* vao_create();
void vao_bind(VAO* self);
void vao_add_buffer(VAO* self, VBO* vbo);

// VBO
VBO *vbo_create(const void* data, unsigned int size);
void vbo_bind(VBO *self);
void vbo_unbind();
void vbo_destroy(VBO *self);
void vbo_push(VBO *self, GLuint count, GLuint type, GLuint normalized);
int getElementTypeSize(GLuint type);

// IBO
IBO *ibo_create(const unsigned int* data, unsigned int count);
void ibo_bind(IBO *self);
void ibo_destroy(IBO *self);



#endif // GLBUFFER_H
