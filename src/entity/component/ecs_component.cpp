#include "ecs_component.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <stdlib.h>

#include <assert.h>
#include <entity/lua/eventstore.hpp>

#if GET_TAG
//#include <entity/ecsdefs.h>
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
    ulong sizeReq = 0;
    for (auto &var : data) {
        var.second.position = sizeReq;
        sizeReq += var.second.size * var.second.stride;
    }

    m_Variables = data;
    m_SizeReq   = sizeReq;
}

#include <entity/v1/ecs.h>
entity::ECSComponentList::ECSComponentList(ECSComponentType componentTypeIndex,
                                           ECSComponentLayout &layout)
    : m_componentLayout(layout)
{
    ECS *ecs      = entity::LuaEventStore::GetInstance().m_ecs;
    ui32 capacity = ecs->capacity;

    // m_componentsList = malloc(sizeof(ECSComponent **) * ECSCom)

    m_components = (ECSComponent **)malloc(sizeof(ECSComponent *) * capacity);
    for (int i = 0; i < capacity; i++) {
        //
        if (ecs_has(
              Entity {.id = (EntityId)i, .index = (EntityId)i, .ecs = ecs},
              componentTypeIndex)) {
            m_components[i] = new ECSComponent(
              (void *)ecs_get(
                Entity {.id = (EntityId)i, .index = (EntityId)i, .ecs = ecs},
                componentTypeIndex),
              layout);
        } else {
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
    if (acr.size) {
        memcpy((char *)m_Data.mem + acr.position, value, acr.size * acr.stride);
        return 1;
    }
    return 0;
}