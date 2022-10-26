#include "component.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <fstream>

#include <stdlib.h>

ecs::Component::Component(ComponentLayout &layout) : m_ComponentLayout(layout) {
    m_Data.mem = malloc((char)layout.getSizeReq());
    m_Data.size = layout.getSizeReq();
}

void ecs::Component::SetVariable(std::string var, void *value) {
    Accessor acr = m_ComponentLayout.getAccessor(var);
    if(acr.size) {
        memcpy((char *)m_Data.mem+acr.position, value, acr.size * acr.stride);
    }
}
