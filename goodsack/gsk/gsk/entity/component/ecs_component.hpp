/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ECS_COMPONENT_HPP__
#define __ECS_COMPONENT_HPP__

#include <map>
#include <string>
#include <type_traits>

#include "util/maths.h"
#include "util/sysdefs.h"

#include "entity/ecs.h"

namespace entity {

enum class EcsDataType {
    INT = 0,
    UINT,
    FLOAT,
    BOOL,
    VEC2,
    VEC3,
    VEC4,
    MAT2,
    MAT3,
    MAT4,
    STRING,
    RESOURCE,
    ENTITY,
};

typedef struct DataTypeContainer_t
{
    int size, stride;
    EcsDataType type;
} DataTypeContainer;

typedef struct Accessor_t
{
    int position, size, stride;
    EcsDataType type;
} Accessor;

// #define DATA_TYPE_TABLE

class ECSComponentLayout {
   public:
    ECSComponentLayout(const char *name);
    ~ECSComponentLayout();

    void SetData(std::map<std::string, Accessor> data);

    std::map<std::string, Accessor> getData() { return m_Variables; };
    Accessor getAccessor(std::string var) { return m_Variables[var]; };
    ulong getSizeReq() { return m_SizeReq; };
    const char *getName() { return m_Name; };

   private:
    std::map<std::string, Accessor> m_Variables;
    ulong m_SizeReq;
    char m_Name[256];
    char m_NameType[256];
};

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
