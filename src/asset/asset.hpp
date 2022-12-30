#ifndef HPP_ASSET
#define HPP_ASSET

#include <util/sysdefs.h>

#include <asset/asset_defs.h>

namespace asset {

class Asset {
   public:
    bool isLoaded() { return m_isLoaded; };
    ui32 getAssetId() { return m_assetId; };

    virtual void Load()   = 0;
    virtual void Unload() = 0;

   protected:
    void setLoaded(bool value) { m_isLoaded = value; };
    AssetType m_assetType;

   private:
    bool m_isLoaded = false;
    ui32 m_assetId;
};

//----------------------------

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

}; // namespace asset

#endif // HPP_ASSET
