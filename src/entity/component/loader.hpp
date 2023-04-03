#ifndef HPP_ECS_LOADER
#define HPP_ECS_LOADER

#include <map>
#include <string>

#include <entity/component/layout.hpp>

#include <util/sysdefs.h>

namespace ecs {

std::map<std::string, ComponentLayout *>
ParseComponents(std::string path, ui32 rawData = 0);

};

#endif // HPP_ECS_LOADER
