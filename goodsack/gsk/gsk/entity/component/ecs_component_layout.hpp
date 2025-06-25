/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ECS_COMPONENT_LAYOUT_HPP__
#define __ECS_COMPONENT_LAYOUT_HPP__

#include <map>
#include <string>

#include "util/sysdefs.h"

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

}; // namespace entity

#endif // __ECS_COMPONENT_LAYOUT_HPP__