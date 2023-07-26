#ifndef H_MODEL
#define H_MODEL

#include <core/graphics/mesh/mesh.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum ModelFileType { OBJ = 0, GLTF } ModelFileType;

#if 0
typedef struct ModelOptions
{
    float scale;
    ui16 import_materials;
} TextureOptions;
#endif

typedef struct Model
{

    const char *modelPath;

    Mesh **meshes;
    ui32 meshesCount;

    ModelFileType fileType;

} Model;

Model *
model_load_from_file(const char *path, f32 scale, ui16 importMaterials);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_MODEL
