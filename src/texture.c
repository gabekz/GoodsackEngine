#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <util/sysdefs.h>

#define TEXTURE_WRAPPING  GL_REPEAT

Texture *texture_create(const char *path, ui32 format,
        ui16 genMipMaps, float afRange) {
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

    glTexImage2D(GL_TEXTURE_2D, 0, format, tex->width, tex->height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);

    // Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TEXTURE_WRAPPING);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TEXTURE_WRAPPING);

    // Mipmaps
    if(genMipMaps >= 0) {
        glGenerateTextureMipmap(tex->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            GL_LINEAR);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // Anistropic Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, afRange);

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
