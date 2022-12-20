#ifndef H_DEVICE_API
#define H_DEVICE_API

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define DEVICE_API_OPENGL device_getGraphics() == GRAPHICS_API_OPENGL
#define DEVICE_API_VULKAN device_getGraphics() == GRAPHICS_API_VULKAN

typedef enum {
    GRAPHICS_API_OPENGL,
    GRAPHICS_API_VULKAN
} GraphicsAPI;

typedef enum {
    OPENAL,
} AudioAPI;


GraphicsAPI device_getGraphics();
void device_setGraphics(GraphicsAPI api);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_DEVICE_API
