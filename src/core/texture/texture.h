#ifndef H_TEXTURE
#define H_TEXTURE

#include <stdlib.h>

#include <util/gfx.h>
#include <util/sysdefs.h>

#include <core/api/vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _texture Texture;

struct _texture
{
    const char *filePath;
    si32 bpp;
    si32 width, height;
    ui32 id;
    ui32 activeSlot;

    struct
    {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;

        VkImageView textureImageView;
        VkSampler textureSampler;
    } vulkan;
};

Texture *
texture_create(const char *path,
               ui32 format,
               ui16 genMipMaps,
               float afRange,
               VulkanDeviceContext *vkDevice);

Texture *
texture_create_cubemap(ui32 faceCount, ...);
Texture *
texture_create_hdr(const char *path);

void
texture_bind(Texture *self, ui32 slot);
void
texture_unbind();

void
texture_cleanup(Texture *self, VulkanDeviceContext *vkDevice);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_TEXTURE
