/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MODEL_ASSET_HPP__
#define __MODEL_ASSET_HPP__

#include "asset/asset.hpp"

namespace goodsack {
namespace asset {

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

} // namespace asset
}; // namespace goodsack

#endif // __MODEL_ASSET_HPP__
