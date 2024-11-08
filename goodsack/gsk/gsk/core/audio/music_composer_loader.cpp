/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "music_composer_loader.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include <stdlib.h>

#include "core/audio/music_composer.h"

#include "util/logger.h"
#include "util/sysdefs.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

gsk_MusicComposer
gsk::audio::composer::create_from_json(std::string full_path)
{

    gsk_MusicComposer ret = {0};

    json JSON;

    std::ifstream file(full_path);
    JSON = json::parse(file);

    for (auto &cmp : JSON.items())
    {
        if (strcmp(cmp.key().c_str(), "Tracks")) { continue; }

        json j_tracks_array = JSON["Tracks"];
        u32 total_tracks    = j_tracks_array.size();

        LOG_INFO("%d", total_tracks);

        for (int i = 0; i < total_tracks; i++)
        {
            json j_track = j_tracks_array[i];

            bool track_name_found = false;
            bool track_path_found = false;

            std::string track_name;
            std::string track_path;

            for (auto &track : j_track.items())
            {
                if (!strcmp(track.key().c_str(), "name"))
                {
                    if (!track.value().is_string())
                    {
                        LOG_CRITICAL("name must be a string!");
                    }

                    track_name_found = true;
                    track_name       = track.value();
                }

                if (!strcmp(track.key().c_str(), "path"))
                {
                    if (!track.value().is_string())
                    {
                        LOG_CRITICAL("name must be a string!");
                    }

                    track_path_found = true;
                    track_path       = track.value();
                }
            }

            if (track_name_found && track_path_found)
            {
                LOG_DEBUG("track: %s | path: %s",
                          track_name.c_str(),
                          track_path.c_str());
            }
        }
    }

    for (auto &cmp : JSON.items())
    {
        if (strcmp(cmp.key().c_str(), "Sequences")) { continue; }
    }

    for (auto &cmp : JSON.items())
    {
        if (strcmp(cmp.key().c_str(), "Stages")) { continue; }
    }

    return ret;
}