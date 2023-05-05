#include "ecs_component_layout.hpp"

#include <stdlib.h>

#include <iostream>
#include <map>
#include <string>

#include <util/maths.h>
#include <util/sysdefs.h>

using namespace entity;

ECSComponentLayout::ECSComponentLayout(const char *name) { m_Name = name; }

void
ECSComponentLayout::SetData(std::map<std::string, Accessor> data)
{
    ulong sizeReq = 0;
    for (auto &var : data) {
        var.second.position = sizeReq;
        sizeReq += var.second.size;
    }

    m_Variables = data;
    m_SizeReq   = sizeReq;
}
