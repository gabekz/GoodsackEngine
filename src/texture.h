#ifndef TEXTURE_H
#define TEXTURE_H

#include "gfx.h"
#include "stdlib.h"


struct Texture {
  const char *filePath;
  unsigned int id;
  unsigned int bpp;
  unsigned int width, height;
  unsigned int slot;
};

struct Texture *texture_create(unsigned char* path);

void texture_bind(struct Texture *self);
void texture_unbind();


#endif
