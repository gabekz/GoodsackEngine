#include "layout.hpp"

#include <stdlib.h>

#include <iostream>
#include <map>
#include <string>

#include <util/maths.h>

using namespace ecs;

ComponentLayout::ComponentLayout(const char *name) { m_Name = name; }

void
ComponentLayout::SetData(std::map<std::string, Accessor> data)
{
    ulong sizeReq = 1; // TODO: Look into this, just to double check.
    for (auto &var : data) {
        var.second.position = sizeReq;
        sizeReq += var.second.size;
    }

    m_Variables = data;
    m_SizeReq   = sizeReq;
}
