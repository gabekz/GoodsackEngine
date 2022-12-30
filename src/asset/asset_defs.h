#ifndef H_ASSET_DEFS
#define H_ASSET_DEFS

namespace asset {

enum class AssetType : int
{
    UNKNOWN = 0,
    SHADER,
    MATERIAL,
    MODEL,
    TEXTURE,
};

enum class TextureFormat { UNKNOWN = 0, RGBA8, RGB8, SRGBA };

};


#endif // H_ASSET_DEFS
