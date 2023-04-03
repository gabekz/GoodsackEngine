#include "component.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <stdlib.h>

entity::Component::Component(ComponentLayout &layout)
    : m_ComponentLayout(layout)
{
    m_Data.mem  = malloc((char)layout.getSizeReq());
    m_Data.size = layout.getSizeReq();
}

int
entity::Component::SetVariable(std::string var, void *value)
{
    Accessor acr = m_ComponentLayout.getAccessor(var);
    if (acr.size) {
        memcpy((char *)m_Data.mem + acr.position, value, acr.size * acr.stride);
        return 1;
    }
    return 0;
}
