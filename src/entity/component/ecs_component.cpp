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
    si32 sizeReq      = 0;
    si32 lastPadding  = 0; // Going to assume padding is 8
    si32 totalPadding = 0;

    for (auto &var : data) {

        var.second.position = sizeReq;

        // last padding is stored, i.e,
        // should be 6 for ui16.
        totalPadding += lastPadding;
        lastPadding =
          ECS_COMPONENTS_ALIGN_BYTES - (var.second.size * var.second.stride);

        // correction for over-done
        if (lastPadding <= 0) lastPadding = 0;

            // add it all together
// TODO: FIX THE ALIGNMENT ISSUE (may only be persistent to
// over-correction)
#if defined(SYS_ENV_WIN)
        (ECS_COMPONENTS_PACKED) ? lastPadding = 0
                                : lastPadding = (lastPadding - 1);
        sizeReq += (var.second.size * var.second.stride) + (lastPadding - 1);
#elif defined(SYS_ENV_UNIX)
        (ECS_COMPONENTS_PACKED) ? lastPadding = 0
                                : lastPadding = (lastPadding + 1);
        sizeReq += (var.second.size * var.second.stride) + (lastPadding);
#endif // SYS_ENV
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
