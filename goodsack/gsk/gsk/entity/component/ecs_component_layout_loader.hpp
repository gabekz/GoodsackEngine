/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ECS_COMPONENT_LAYOUT_LOADER_HPP__
#define __ECS_COMPONENT_LAYOUT_LOADER_HPP__

#include <map>
#include <string>
#include <vector>

#include "entity/component/ecs_component_layout.hpp"
#include "util/sysdefs.h"

namespace entity {
namespace component {

typedef std::map<std::string, int> ComponentLayoutMap;
typedef std::vector<ECSComponentLayout *> ComponentLayoutsContainer;

void
parse_components_from_json(ComponentLayoutMap &layouts_map,
                           ComponentLayoutsContainer &layouts_container,
                           std::string path,
                           std::string scheme,
                           u32 rawData = 0);

bool
generate_cpp_types(std::string path,
                   ComponentLayoutsContainer layouts_container);

}; // namespace component

}; // namespace entity

#endif // __ECS_COMPONENT_LAYOUT_LOADER_HPP__