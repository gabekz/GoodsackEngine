#include "ibo.h"

struct IBO *ibo_create(const unsigned int* data, unsigned int size)
{
  struct IBO *ibo = malloc(sizeof(struct IBO)); 

   glGenBuffers(1, &ibo->id);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->id);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

   return ibo;
}

void ibo_bind(struct IBO *self)
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->id);
}

void ibo_destroy(struct IBO *self)
{
   glDeleteBuffers(1, &self->id);

}

void ibo_unbind()
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



