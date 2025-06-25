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
