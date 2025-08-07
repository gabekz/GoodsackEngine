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

#define CHECK_KEY(x, str) (!strcmp(x.key().c_str(), str))

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

        for (auto &seq : JSON["Sequences"].items())
        {
            if (!seq.value().is_array()) { LOG_CRITICAL("Must be array"); }

            for (auto &segment : seq.value().items())
            {
                if (!segment.value().is_object())
                {
                    LOG_CRITICAL("Must be an object");
                }

                // get track_name, start time
                for (auto &track_item : segment.value().items())
                {
                    if (!strcmp(track_item.key().c_str(), "track"))
                    {
                        std::string name = track_item.value();
                        LOG_INFO("Track name %s", name.c_str());
                    }
                    if (!strcmp(track_item.key().c_str(), "start_time"))
                    {
                        u32 start_time = track_item.value();
                        LOG_INFO("start time %d", start_time);
                    }
                }
            }
        }
    }

    for (auto &cmp : JSON.items())
    {
        if (strcmp(cmp.key().c_str(), "Stages")) { continue; }

        u32 bpm             = 120;
        u32 beats_per_bar   = 4;
        u32 bars_per_phrase = 4;

        gsk_ComposerStage stage = {};

        for (auto &stage : cmp.value().items())
        {
            if (!stage.value().is_object())
            {
                LOG_CRITICAL("Stage must be an object!");
            }

            for (auto &stage_item : stage.value().items())
            {
                if (CHECK_KEY(stage_item, "bpm")) { LOG_INFO("BPM"); }

                if (CHECK_KEY(stage_item, "bars_per_phrase"))
                {
                    LOG_INFO("BPP");
                }

                if (CHECK_KEY(stage_item, "composition"))
                {
                    if (!stage_item.value().is_array())
                    {
                        LOG_CRITICAL("composition must be an array");
                    }

                    for (auto &comp_seq : stage_item.value().items())
                    {
                        std::string sequence = comp_seq.value();
                        LOG_TRACE("Sequence: %s", sequence.c_str());
                        // add sequence here
                    }
                }
            }
        }
    }

    return ret;
}