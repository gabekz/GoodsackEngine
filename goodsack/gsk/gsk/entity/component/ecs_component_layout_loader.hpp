/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ECS_COMPONENT_LAYOUT_LOADER_HPP__
#define __ECS_COMPONENT_LAYOUT_LOADER_HPP__

#include <map>
#include <string>

#include "entity/component/ecs_component.hpp"
#include "util/sysdefs.h"

namespace entity {
namespace component {

typedef std::map<std::string, ECSComponentLayout *> ComponentLayoutMap;

ComponentLayoutMap
parse_components_from_json(std::string path, u32 rawData = 0);

int
generate_cpp_types(std::string path, ComponentLayoutMap map);

}; // namespace component

}; // namespace entity

#endif // __ECS_COMPONENT_LAYOUT_LOADER_HPP__