#include "ecs_component_layout.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <assert.h>
#include <stdlib.h>

#include "entity/ecsdefs.h"

entity::ECSComponentLayout::ECSComponentLayout(const char *name)
{
    strcpy(m_Name, name);

#if 0
    char newNameType[256];
    strcpy(newNameType, name);
    std::string newNameType_C = "C_";
    newNameType_C.append(newNameType);
    strcpy(m_NameType, newNameType_C.c_str());
#endif
}

void
entity::ECSComponentLayout::SetData(std::map<std::string, Accessor> data)
{
    s32 sizeReq      = 0;
    s32 lastPadding  = 0; // Going to assume padding is 8
    s32 totalPadding = 0;

    for (auto &var : data)
    {

        var.second.position = sizeReq;

        // last padding is stored, i.e,
        // should be 6 for u16.
        totalPadding += lastPadding;

        s32 varMemSize = var.second.size * var.second.stride;

        lastPadding = (varMemSize < ECS_COMPONENTS_ALIGN_BYTES)
                        ? ECS_COMPONENTS_ALIGN_BYTES
                        : varMemSize;

        // Add by padding
        sizeReq += lastPadding;
    }

    m_Variables = data;
    m_SizeReq   = sizeReq;
}
