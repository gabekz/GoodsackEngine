/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "ecs_component.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <assert.h>
#include <stdlib.h>

#include "entity/lua/eventstore.hpp"

#include "entity/ecs.h"

#if GET_TAG
// #include <entity/ecsdefs.h>
#endif

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

entity::ECSComponentList::ECSComponentList(ECSComponentType componentTypeIndex,
                                           ECSComponentLayout &layout)
    : m_componentLayout(layout)
{
    gsk_ECS *ecs = entity::LuaEventStore::GetInstance().m_ecs;
    u32 capacity = ecs->capacity;

    // m_componentsList = malloc(sizeof(ECSComponent **) * ECSCom)

    m_components = (ECSComponent **)malloc(sizeof(ECSComponent *) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        //
        if (gsk_ecs_has(gsk_Entity {.id    = (gsk_EntityId)i,
                                    .index = (gsk_EntityId)i,
                                    .ecs   = ecs},
                        componentTypeIndex))
        {
            m_components[i] = new ECSComponent(
              (void *)gsk_ecs_get(gsk_Entity {.id    = (gsk_EntityId)i,
                                              .index = (gsk_EntityId)i,
                                              .ecs   = ecs},
                                  componentTypeIndex),
              layout);
        } else
        {
            m_components[i] = new ECSComponent(layout);
        }
    }
}

entity::ECSComponent::ECSComponent(ECSComponentLayout &layout)
    : m_componentLayout(layout)
{
    m_Data.mem  = malloc(layout.getSizeReq());
    m_Data.size = layout.getSizeReq();
}
entity::ECSComponent::ECSComponent(void *pData, ECSComponentLayout &layout)
    : m_componentLayout(layout)
{
    m_Data.mem  = pData;
    m_Data.size = layout.getSizeReq();
#if GET_TAG
    m_Data.tag = *(char *)m_Data.mem - ECS_TAG_SIZE;
#endif
}

int
entity::ECSComponent::SetVariable(std::string var, void *value)
{
    Accessor acr = m_componentLayout.getAccessor(var);
    if (acr.size)
    {
        memcpy((char *)m_Data.mem + acr.position, value, acr.size * acr.stride);
        return 1;
    }
    return 0;
}
