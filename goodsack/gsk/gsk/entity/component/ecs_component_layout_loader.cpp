/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

/* TODO: Parser    - ordering + sub-struct
   TODO: Parser    - strings (ECSString a.k.a. char * const)
   TODO: Generator - ordering
   TODO: This whole thing is a damn mess..
*/

#include "ecs_component_layout_loader.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <stdlib.h>

#include "util/logger.h"
#include "util/maths.h"

#include "entity/component/ecs_component.hpp"

// TODO: Move to thirdparty directive - gabekz/GoodsackEngine#19
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::string
_pascal_to_snake_upper(const std::string &input)
{
    std::string result;
    result.reserve(input.size() +
                   5); // rough reserve to avoid frequent reallocs

    for (size_t i = 0; i < input.size(); ++i)
    {
        char c       = input[i];
        bool isUpper = std::isupper(static_cast<unsigned char>(c));
        bool prevIsLower =
          (i > 0 && std::islower(static_cast<unsigned char>(input[i - 1])));
        bool nextIsLower =
          (i + 1 < input.size() &&
           std::islower(static_cast<unsigned char>(input[i + 1])));

        if (isUpper && i > 0 && (prevIsLower || nextIsLower)) { result += '_'; }

        result +=
          static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    return result;
}

// TODO: take in a map rather than creating one here
// TODO: check if this is a file rather that rawData param
entity::component::ComponentLayoutMap
entity::component::parse_components_from_json(std::string path, u32 rawData)
{
    json JSON;

    if (rawData <= 0)
    {
        std::ifstream file(path);
        JSON = json::parse(file);
    } else
    {
        JSON = json::parse(path);
    }

    /* ----------------------------------------------------------
        Data Types table                                       */

    std::map<std::string, DataTypeContainer> dataTypes = {
      // Numeric
      {"int", (DataTypeContainer {sizeof(int), 1, EcsDataType::INT})},
      {"uint", (DataTypeContainer {sizeof(u32), 1, EcsDataType::UINT})},
      {"float", (DataTypeContainer {sizeof(float), 1, EcsDataType::FLOAT})},
      {"bool", (DataTypeContainer {sizeof(u16), 1, EcsDataType::BOOL})},
      // Vector
      {"vec2", (DataTypeContainer {sizeof(float), 2, EcsDataType::VEC2})},
      {"vec3", (DataTypeContainer {sizeof(float), 3, EcsDataType::VEC3})},
      {"vec4", (DataTypeContainer {sizeof(float), 4, EcsDataType::VEC4})},
      // Matrix
      {"mat2", (DataTypeContainer {sizeof(float[2]), 2, EcsDataType::MAT2})},
      {"mat3", (DataTypeContainer {sizeof(float[3]), 3, EcsDataType::MAT3})},
      {"mat4", (DataTypeContainer {sizeof(float[4]), 4, EcsDataType::MAT4})},
      // String - TODO: implement correctly
      {"string",
       (DataTypeContainer {sizeof(const char *), 1, EcsDataType::STRING})},
      // Resource reference (void ptr)
      {"Resource",
       (DataTypeContainer {sizeof(void *), 1, EcsDataType::RESOURCE})},
      {"entity", (DataTypeContainer {sizeof(int), 1, EcsDataType::ENTITY})},
    };

    // ----------------------------------------------------------

    std::map<std::string, ECSComponentLayout *> layouts;

    // Loop through every component
    for (auto &cmp : JSON.items())
    {
        // Component Layout
        ECSComponentLayout *component =
          new ECSComponentLayout(cmp.key().c_str());

        // Component Layout Data
        std::map<std::string, Accessor> data;

        // Every type of available data
        for (auto type : dataTypes)
        {
            // TODO: Switch loop to first check all data in the ComponentLayout
            // rather than all dataTypes.

            json JData = JSON[cmp.key()][type.first];
            // Every variable of 'type' in the component
            for (int i = 0; i < JData.size(); i++)
            {
                if (!strcmp(type.first.c_str(), "string"))
                {
                    // TODO: handle
                    // data[JData[i]] = (Accessor){0, type.second.size,
                    // type.second.stride};
                    LOG_WARN("String is not implemented correctly");
                    // continue;
                }

                // TODO: just use DataType inside of accessor...
                data[JData[i]] = (Accessor {
                  0, type.second.size, type.second.stride, type.second.type});
                // std::cout << type.first << ": " << JData[i] << std::endl;
            }
        }

        component->SetData(data);
        layouts[cmp.key()] = component; // i.e, layouts["ComponentTransform"]
    }

    generate_cpp_types("test.h", layouts);
    return layouts;
}

#define GEN_USING_TYPEDEF 1

int
entity::component::generate_cpp_types(std::string path,
                                      entity::component::ComponentLayoutMap map)
{
    std::ostringstream buff;

    // Setup file for writing
    // freopen(path.c_str(), "w", stdout);

    // Header
    std::string header = R"(
// @generated file

#ifndef H_COMPONENTS_GEN
#define H_COMPONENTS_GEN

#include <util/maths.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

//typedef (void *)(ResRef)
#define ResRef void *

        )";

    // Write header
    buff << header << "\n";

    // Create ECS Component ENUM
    buff << "typedef enum ECSComponentType_t {\n";

    int lastComponentIndex = -1;
    for (const auto &p : map)
    {
#if 0
        std::string componentName = p.first;
        std::for_each(componentName.begin(), componentName.end(), [](char &c) {
            c = ::toupper(c);
        });
#else
        std::string componentName = _pascal_to_snake_upper(p.first);
#endif

        buff << "C_" << componentName << ",\n";
        lastComponentIndex++;
    }
    buff << "} ECSComponentType;\n\n";

    // store last component index
    buff << "#define ECSCOMPONENT_LAST " << lastComponentIndex << "\n\n";

    // Create struct for each ECS Component Type
    for (const auto &p : map)
    {
        std::string componentName = _pascal_to_snake_upper(p.first);

#if GEN_USING_TYPEDEF
        buff << "typedef struct "
             << "Component" << p.first << " { \n";
#else
        buff << "struct "
             << "Component" << p.first << " {\n";
#endif

        ECSComponentLayout *layout = map[p.first];
        int lastPosition           = 0;

        // Create struct data
        // TODO: Ordering
        for (const auto &q : layout->getData())
        {
            Accessor accessor = layout->getData()[q.first];

            // Check to make sure the order is correct
            assert((accessor.position != 0) ? accessor.position > lastPosition
                                            : lastPosition == 0);
            lastPosition = accessor.position;

            buff << "\tCACHE_ALIGN(";

            // TODO: change to inline converter
            switch (accessor.type /* accessor.type */)
            {
            case EcsDataType::INT: buff << "s32 "; break;
            case EcsDataType::UINT: buff << "u32 "; break;
            case EcsDataType::FLOAT: buff << "f32 "; break;
            case EcsDataType::BOOL: buff << "u16 "; break;
            case EcsDataType::VEC2: buff << "vec2 "; break;
            case EcsDataType::VEC3: buff << "vec3 "; break;
            case EcsDataType::VEC4: buff << "vec4 "; break;
            case EcsDataType::MAT3: buff << "mat3 "; break;
            case EcsDataType::MAT4: buff << "mat4 "; break;

            // TODO: implement strings correctly
            case EcsDataType::STRING: buff << "const char *"; break;

            case EcsDataType::RESOURCE: buff << "ResRef "; break;
            case EcsDataType::ENTITY: buff << "int "; break;
            default: break;
            }
            buff << q.first << ");\n";
        }

// Close struct
#if GEN_USING_TYPEDEF
        buff << "} "
             << "gsk_C_" << p.first << ";\n\n";
#else
        buff << "};\n\n";
#endif
    }
    // Footer
    std::string footer = R"(

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //H_COMPONENTS_GEN

)";

    // Write footer
    buff << footer << "\n";

    // PART 2 - components_gen_register.h
    // Header2
    std::string header2 = R"(
#ifdef COMPONENTS_GEN_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

        )";

    // Write header
    buff << header2 << "\n";

    // Create initializer Component Register
    buff << "static inline void\n_ecs_init_internal_gen(gsk_ECS *ecs) {\n";

    for (const auto &p : map)
    {
        std::string componentName = _pascal_to_snake_upper(p.first);

        buff << "\t_ECS_DECL_COMPONENT_INTERN(ecs, C_" << componentName
             << ", sizeof(struct Component" << p.first << "));\n";
    }
    buff << "}\n";

#if 0
    // Lua Component Register - TODO: Rework
    std::cout << "// Lua Component Register" << std::endl;
    std::cout << "ifdef __cplusplus\n" << std::endl;
    std::cout << "static inline void _ecs_lua_internal_cmp_register() {"
              << std::endl;
    std::cout << "}\n" << std::endl;
    std::cout << "#endif //__cplusplus" << std::endl;
#endif

    // Footer2
    std::string footer2 = R"(

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //COMPONENTS_GEN_IMPLEMENTATION

        )";
    // Write footer
    buff << footer2;

    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out)
    {
        LOG_CRITICAL("Failed to open %s", path.c_str());
        return 0;
    }

    out << buff.str();

    return 1;
}
