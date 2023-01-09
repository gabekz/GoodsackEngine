#ifndef HPP_TEXTURE_ASSET
#define HPP_TEXTURE_ASSET

#include <asset/asset.hpp>
#include <util/sysdefs.h>

namespace goodsack { namespace asset {

enum class TextureFormat { UNKNOWN = 0, RGBA8, RGB8, SRGBA };

struct TextureProperties
{
    const char *filePath;
    si32 bpp;
    si32 width, height;
    TextureFormat format = TextureFormat::SRGBA;
};

class TextureAsset : public Asset {
   public:
    TextureAsset(TextureProperties props);

    void Load();
    void Unload();

   private:
    TextureProperties m_properties;
};

}} // namespace

#endif
