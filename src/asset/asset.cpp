#include "asset.hpp"

#include <util/logger.h>

namespace asset {

TextureAsset::TextureAsset(TextureProperties props) {
    m_properties = props;
    setLoaded(false);
}

void TextureAsset::Load() {
    LOG_DEBUG("Hello! %d", m_properties.bpp);
}

void TextureAsset::Unload() {

}

};
