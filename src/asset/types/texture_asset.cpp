#include "texture_asset.hpp"

#include <util/logger.h>

namespace goodsack { namespace asset {

TextureAsset::TextureAsset(TextureProperties props)
{
    m_properties = props;
    SetLoaded(false);
}

void
TextureAsset::Load()
{
    SetLoaded(true);
}

void
TextureAsset::Unload()
{
}

}};
