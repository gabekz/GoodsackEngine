/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __TEXTURE_ASSET_HPP__
#define __TEXTURE_ASSET_HPP__

#include "util/sysdefs.h"
#include "asset/asset.hpp"

namespace goodsack {
namespace asset {

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

} // namespace asset
} // namespace goodsack

#endif // __TEXTURE_ASSET_HPP__
