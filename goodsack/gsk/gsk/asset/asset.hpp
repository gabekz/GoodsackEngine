#ifndef HPP_ASSET
#define HPP_ASSET

#include <util/sysdefs.h>

namespace goodsack {
namespace asset {

enum class AssetType : int {
    UNKNOWN = 0,
    SHADER,
    MATERIAL,
    MODEL,
    TEXTURE,
};

class Asset {
   public:
    bool IsLoaded() { return m_isLoaded; };
    ui32 GetAssetId() { return m_assetId; };

    virtual void Load()   = 0;
    virtual void Unload() = 0;

   protected:
    void SetLoaded(bool value) { m_isLoaded = value; };
    AssetType m_assetType;

   private:
    bool m_isLoaded = false;
    ui32 m_assetId;
}; // class

} // namespace asset
}; // namespace goodsack

#endif // HPP_ASSET
