#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TEXTURE_REPEAT

Texture *texture_create(unsigned char* path) {
    Texture *tex = malloc(sizeof(Texture));
    tex->filePath = path;
    tex->slot = 0;

    stbi_set_flip_vertically_on_load(1);

    unsigned char* localBuffer = stbi_load(tex->filePath, &tex->width,
      &tex->height, &tex->bpp, /*RGBA*/ 4);

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#ifdef TEXTURE_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width, tex->height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);

    glBindTexture(GL_TEXTURE_2D, 0);

    if(localBuffer) {
      stbi_image_free(localBuffer);
    }

    return tex;
}

void texture_bind(Texture *self) {
  glActiveTexture(GL_TEXTURE0 + self->slot);
  glBindTexture(GL_TEXTURE_2D, self->id);
}

void texture_unbind() {
  glBindTexture(GL_TEXTURE_2D, 0);
}
