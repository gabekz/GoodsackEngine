#ifndef TEXTURE_H
#define TEXTURE_H

#include "gfx.h"
#include "stdlib.h"
#include <util/sysdefs.h>

typedef struct _texture Texture;

struct _texture {
  const char *filePath;
  si32 bpp;
  si32 width, height;
  ui32 id;
  ui32 slot;
};

Texture *texture_create(const char *path, ui32 slot);

void texture_bind(Texture *self);
void texture_unbind();

#endif
