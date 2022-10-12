#include "../texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdarg.h>
#include <util/sysdefs.h>

#define TEXTURE_WRAPPING  GL_REPEAT

Texture *texture_create(const char *path, ui32 format,
        ui16 genMipMaps, float afRange) {
    Texture *tex = malloc(sizeof(Texture));
    tex->filePath = path;

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    stbi_set_flip_vertically_on_load(1);
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
    if (afRange > 0) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, afRange);
    }

    if(localBuffer) {
      stbi_image_free(localBuffer);
    }

    return tex;
}

Texture *texture_create_cubemap(ui32 faceCount, ...) {
    ui32 textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    Texture *tex = malloc(sizeof(Texture));
    tex->id = textureId;

    va_list ap;
    va_start(ap, faceCount);
    va_end(ap);

    stbi_set_flip_vertically_on_load(0);
    for(int i = 0; i < faceCount; i++) {
        unsigned char *data;
        const char *path = va_arg(ap, const char*);
        if(path != NULL) {
            data = stbi_load(path, &tex->width,
            &tex->height, &tex->bpp, /*RGBA*/ 0);

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                tex->width, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, data 
            );
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return tex;
}

Texture *texture_create_hdr(const char *path) {
    Texture *tex = malloc(sizeof(Texture));

    float *data = stbi_loadf(path,
        &tex->width, &tex->height, &tex->bpp, 0);

    assert(data != NULL);

    ui32 textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
        tex->width, tex->height, 0, GL_RGB, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    tex->id = textureId;
    tex->filePath = path;

    free(data);
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
