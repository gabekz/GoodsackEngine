#ifndef HPP_ECS_COMPONENT_LAYOUT_LOADER
#define HPP_ECS_COMPONENT_LAYOUT_LOADER

#include <map>
#include <string>

#include <entity/component/ecs_component.hpp>

#include <util/sysdefs.h>

namespace entity {
namespace component {

typedef std::map<std::string, ECSComponentLayout *> ComponentLayoutMap;

ComponentLayoutMap
parse_components_from_json(std::string path, ui32 rawData = 0);

int
generate_cpp_types(std::string path, ComponentLayoutMap map);

}; // namespace component

}; // namespace entity

#endif // HPP_ECS_COMPONENT_LAYOUT_LOADER
