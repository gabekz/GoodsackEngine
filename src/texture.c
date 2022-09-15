#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <util/sysdefs.h>

#define TEXTURE_WRAPPING  GL_REPEAT

Texture *texture_create(const char *path) {
    Texture *tex = malloc(sizeof(Texture));
    tex->filePath = path;

    stbi_set_flip_vertically_on_load(1);

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    unsigned char *localBuffer;
    if(path != NULL) {
        localBuffer = stbi_load(tex->filePath, &tex->width,
        &tex->height, &tex->bpp, /*RGBA*/ 4);
    }
    else {
        localBuffer = NULL;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width, tex->height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TEXTURE_WRAPPING);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TEXTURE_WRAPPING);

    glBindTexture(GL_TEXTURE_2D, 0);

    if(localBuffer) {
      stbi_image_free(localBuffer);
    }

    return tex;
}

void texture_bind(Texture *self, ui32 slot) {
    self->activeSlot = slot;
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, self->id);
}

void texture_unbind() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
