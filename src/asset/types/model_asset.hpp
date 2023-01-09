#ifndef HPP_MODEL_ASSET
#define HPP_MODEL_ASSET

#include <asset/asset.hpp>

namespace goodsack { namespace asset {

struct ModelProperties
{
    const char *filePath;
};

class ModelAsset : public Asset {
public:
    ModelAsset(ModelProperties props);

    void Load();
    void Unload();

private:
    ModelProperties m_properties;
};

}};

#endif
