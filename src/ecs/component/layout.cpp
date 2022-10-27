#include "layout.hpp"

#include <stdlib.h>

#include <iostream>
#include <string>
#include <map>

#include <util/maths.h>

using namespace ecs;

ComponentLayout::ComponentLayout(const char *name) { m_Name = name; }

void ComponentLayout::SetData(std::map<std::string, Accessor> data) {
    ulong sizeReq = 0;
    for(auto& var : data) {
        var.second.position = sizeReq;
        sizeReq += var.second.size;
    }

    m_Variables = data;
    m_SizeReq   = sizeReq;
}
