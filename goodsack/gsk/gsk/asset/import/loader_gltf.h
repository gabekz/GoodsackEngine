#ifndef H_LOADER_GLTF
#define H_LOADER_GLTF

#include <core/graphics/mesh/mesh.h>
#include <core/graphics/mesh/model.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

Model *
load_gltf(const char *path, int scale, int importMaterials);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_LOADER_GLTF
