#ifndef HPP_ECS_COMPONENT_LAYOUT_LOADER
#define HPP_ECS_COMPONENT_LAYOUT_LOADER

#include <map>
#include <string>

#include <entity/component/ecs_component_layout.hpp>

#include <util/sysdefs.h>

namespace entity {

std::map<std::string, ECSComponentLayout *>
ParseComponents(std::string path, ui32 rawData = 0);

}; // namespace entity

#endif // HPP_ECS_COMPONENT_LAYOUT_LOADER
