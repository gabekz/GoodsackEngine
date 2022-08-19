#ifndef _VAO_H_
#define _VAO_H_

#include "gfx.h"
#include "vbo.h"

typedef struct _VAO VAO;

struct _VAO {
   unsigned int id;
   BufferElement *elements;
   unsigned int elementsCount;
};

VAO* vao_create();
void vao_bind(VAO* self);
void vao_add_buffer(VAO* self, VBO* vbo);

#endif
