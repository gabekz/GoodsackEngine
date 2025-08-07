/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "core/audio/music_composer.h"

#include <string>

namespace gsk {
namespace audio {

namespace composer {

gsk_MusicComposer
create_from_json(std::string full_path);

}
} // namespace audio
} // namespace gsk
