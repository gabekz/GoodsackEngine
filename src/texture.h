#ifndef TEXTURE_H
#define TEXTURE_H

#include "gfx.h"
#include "stdlib.h"

typedef struct _texture Texture;

struct _texture {
  const char *filePath;
  unsigned int id;
  unsigned int bpp;
  unsigned int width, height;
  unsigned int slot;
};

Texture *texture_create(unsigned char* path);

void texture_bind(Texture *self);
void texture_unbind();


#endif
