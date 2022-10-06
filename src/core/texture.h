#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdlib.h>

#include <util/gfx.h>
#include <util/sysdefs.h>

typedef struct _texture Texture;

struct _texture {
  const char *filePath;
  si32 bpp;
  si32 width, height;
  ui32 id;
  ui32 activeSlot;
};

Texture *texture_create(const char *path, ui32 format,
        ui16 genMipMaps, float afRange);

Texture *texture_create_cubemap(ui32 faceCount, ...);

void texture_bind(Texture *self, ui32 slot);
void texture_unbind();

#endif
