/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ECS_COMPONENT_HPP__
#define __ECS_COMPONENT_HPP__

#include <map>
#include <string>
#include <type_traits>

#include "util/maths.h"
#include "util/sysdefs.h"

#include "entity/component/ecs_component_layout.hpp"
#include "entity/ecs.h"

namespace entity {

// #define DATA_TYPE_TABLE

class ECSComponent {
   public:
    ECSComponent(ECSComponentLayout &layout);
    ECSComponent(void *pData, ECSComponentLayout &layout);

    const char *getName() { return m_componentLayout.getName(); };

    int SetVariable(std::string var, void *value);

    template <typename T>
    int GetVariable(std::string var, T *destination);

    EcsDataType GetVariableType(std::string var)
    {
        return m_componentLayout.getAccessor(var).type;
    };

   private:
    struct
    {
        void *mem;
        int size, index;
        char tag; // TODO: -testing
    } m_Data;
    ECSComponentLayout &m_componentLayout;
};

// TODO: This is a really fucking stupid class. Should not
// exist (at least not in this way). Slow as fuck, and eats a lot
// of memory
class ECSComponentList {
   public:
    ECSComponentList(ECSComponentType componentTypeIndex,
                     ECSComponentLayout &layout);

    ECSComponentLayout &m_componentLayout;
    ECSComponent **m_components; // TODO: This is what I am talking about.
};

} // namespace entity

template <typename T>
int
entity::ECSComponent::GetVariable(std::string var, T *destination)
{
    Accessor acr = m_componentLayout.getAccessor(var);
    if (acr.size)
    {
        // printf("TEST T: %f", *(T *)((char *)m_Data.mem+acr.position));
        *destination = *(T *)((char *)m_Data.mem + (acr.position));
        return 1;
    }
    return 0;
}

template <>
inline // vec3 specialization
  int
  entity::ECSComponent::GetVariable<float[3]>(std::string var,
                                              float (*destination)[3])
{
    Accessor acr = m_componentLayout.getAccessor(var);
    if (acr.size)
    {
        glm_vec3_copy((float *)((char *)m_Data.mem + acr.position),
                      *destination);
        return 1;
    }
    return 0;
}

template <>
inline // mat3 specialization
  int
  entity::ECSComponent::GetVariable<float[3][3]>(std::string var,
                                                 float (*destination)[3][3])
{
    Accessor acr = m_componentLayout.getAccessor(var);
    if (acr.size)
    {
        glm_mat3_copy((vec3 *)((char *)m_Data.mem + acr.position),
                      *destination);
        return 1;
    }
    return 0;
}

template <>
inline // mat4 specialization
  int
  entity::ECSComponent::GetVariable<float[4][4]>(std::string var,
                                                 float (*destination)[4][4])
{
    Accessor acr = m_componentLayout.getAccessor(var);
    if (acr.size)
    {
        glm_mat4_copy((vec4 *)((char *)m_Data.mem + acr.position),
                      *destination);
        return 1;
    }
    return 0;
}

#endif // __ECS_COMPONENT_HPP__
