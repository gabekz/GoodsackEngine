#ifndef H_LOADER_GLTF
#define H_LOADER_GLTF

#include <core/graphics/mesh/mesh.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

MeshData *
load_gltf(const char *path, int scale);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_LOADER_GLTF