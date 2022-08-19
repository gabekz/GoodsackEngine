#ifndef IBO_H 
#define IBO_H

#include "gfx.h"
#include "stdlib.h"

struct IBO 
{
   unsigned int id;
   unsigned int count;
};

struct IBO *ibo_create(const unsigned int* data, unsigned int count);

void ibo_bind(struct IBO *self);
void ibo_destroy(struct IBO *self);


#endif
