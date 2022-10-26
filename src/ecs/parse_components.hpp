#ifndef H_PARSE_COMPONENTS
#define H_PARSE_COMPONENTS

#include <string>
#include <map>

#include <ecs/component_layout.hpp>

namespace ecs {

std::map<std::string, ComponentLayout*>
    ParseComponents(const char *path);

};

#endif // H
