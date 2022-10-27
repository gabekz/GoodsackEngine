#ifndef HPP_ECS_LOADER
#define HPP_ECS_LOADER

#include <string>
#include <map>

#include <ecs/component/layout.hpp>

namespace ecs {

std::map<std::string, ComponentLayout*>
    ParseComponents(const char *path);

};

#endif // HPP_ECS_LOADER
