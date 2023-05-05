#include "ecs_component.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <stdlib.h>

entity::ECSComponent::ECSComponent(ECSComponentLayout &layout)
    : m_componentLayout(layout)
{
    m_Data.mem  = malloc((char)layout.getSizeReq());
    m_Data.size = layout.getSizeReq();
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

void
entity::ECSComponent::MapFromExisting(void *value, ECSComponentLayout &layout)
{
    m_Data.mem = value;
    m_Data.size = layout.getSizeReq();

    m_componentLayout = layout;
}
