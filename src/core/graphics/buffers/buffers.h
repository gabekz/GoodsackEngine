#ifndef H_BUFFERS
#define H_BUFFERS

#include <util/gfx.h>
#include <util/sysdefs.h>

enum BufferType = {VERTEX = 0, INDEX, UNIFORM};

struct GraphicsBuffer
{
    BufferType type;
    void *data;
    ui32 length;

    // VkBuffer bufferId;
    // ui32 bufferId;
};

GraphicsBuffer *
graphics_buffer_create(BufferType type, void *data, ui32 size);

#endif // H_BUFFERS