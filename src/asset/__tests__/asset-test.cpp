#include <gtest/gtest.h>
#include <stdlib.h>

#include <asset/asset.hpp>
#include <asset/types/texture_asset.hpp>

using namespace goodsack::asset;

TEST(AssetTestSuite, ReferenceInherited)
{
    TextureAsset texture =
        TextureAsset((TextureProperties) {.bpp = 4});

    Asset *p = &texture;
    EXPECT_EQ(p->IsLoaded(), false);
    p->Load();
    EXPECT_EQ(p->IsLoaded(), true);
}
