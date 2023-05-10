#include "ecs_component_layout_loader.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <util/maths.h>

#include <nlohmann/json.hpp>
#include <stdlib.h>

#include <entity/component/ecs_component.hpp>

using json = nlohmann::json;

entity::component::ComponentLayoutMap
entity::component::parse_components_from_json(std::string path, ui32 rawData)
{
    json JSON;

    if (rawData <= 0) {
        std::ifstream file(path);
        JSON = json::parse(file);
    } else {
        JSON = json::parse(path);
    }

    /* ----------------------------------------------------------
        Data Types table                                       */

    std::map<std::string, DataTypeContainer> dataTypes = {
      // Numeric
      {"int", (DataTypeContainer {sizeof(int), 1, EcsDataType::INT})},
      {"uint", (DataTypeContainer {sizeof(ui32), 1, EcsDataType::UINT})},
      {"float", (DataTypeContainer {sizeof(float), 1, EcsDataType::FLOAT})},
      {"bool", (DataTypeContainer {sizeof(char), 1, EcsDataType::BOOL})},
      // Vector
      {"vec2", (DataTypeContainer {sizeof(float), 2, EcsDataType::VEC2})},
      {"vec3", (DataTypeContainer {sizeof(float), 3, EcsDataType::VEC3})},
      {"vec4", (DataTypeContainer {sizeof(float), 4, EcsDataType::VEC4})},
      // Matrix
      {"mat2", (DataTypeContainer {sizeof(float[2]), 2, EcsDataType::MAT2})},
      {"mat3", (DataTypeContainer {sizeof(float[3]), 3, EcsDataType::MAT3})},
      {"mat4", (DataTypeContainer {sizeof(float[4]), 4, EcsDataType::MAT4})},
      // Resource reference (void ptr)
      {"Resource",
       (DataTypeContainer {sizeof(void *), 1, EcsDataType::RESOURCE})},
    };

    // ----------------------------------------------------------

    std::map<std::string, ECSComponentLayout *> layouts;

    // Loop through every component
    for (auto &cmp : JSON.items()) {

        // Component Layout
        ECSComponentLayout *component =
          new ECSComponentLayout(cmp.key().c_str());

        // Component Layout Data
        std::map<std::string, Accessor> data;

        // Every type of available data
        for (auto type : dataTypes) {
            // TODO: Switch loop to first check all data in the ComponentLayout
            // rather than all dataTypes.

            json JData = JSON[cmp.key()][type.first];
            // Every variable of 'type' in the component
            for (int i = 0; i < JData.size(); i++) {

                if (!strcmp(type.first.c_str(), "string")) {
                    // TODO: handle
                    // data[JData[i]] = (Accessor){0, type.second.size,
                    // type.second.stride};
                    continue;
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

int
entity::component::generate_cpp_types(std::string path,
                                      entity::component::ComponentLayoutMap map)
{
    // Setup file for writing
    // freopen(path.c_str(), "w", stdout);

    // Header
    std::string header = R"(
// @generated file

#include <util/maths.h>
#include <util/sysdefs.h>

//#ifdef __cplusplus
//extern "C" {
//#endif // __cplusplus

typedef (void *)(ResRef)

        )";

    // Write header
    std::cout << header << std::endl;

    // Create ECS Component ENUM
    std::cout << "typedef enum _ecs_component_types {" << std::endl;
    for (const auto &p : map) {
        std::string componentName = p.first;
        std::for_each(componentName.begin(), componentName.end(), [](char &c) {
            c = ::toupper(c);
        });
        std::cout << "C_" << componentName << "," << std::endl;
    }
    std::cout << "} ECSComponentTypes;\n" << std::endl;

    // Create struct for each ECS Component Type
    for (const auto &p : map) {
        // std::cout << p.first << '\t' << p.second << std::endl;

        std::string componentName = p.first;
        // std::cout << "struct " << componentName << " {\n};" << std::endl;
        std::cout << "typedef struct "
                  << "Component" << componentName << "_t { " << std::endl;

        ECSComponentLayout *layout = map[componentName];
        int lastPosition           = 0;

        // Create struct data
        for (const auto &q : layout->getData()) {
            Accessor accessor = layout->getData()[q.first];

            // Check to make sure the order is correct
            assert((accessor.position != 0) ? accessor.position > lastPosition
                                            : lastPosition == 0);
            lastPosition = accessor.position;

            std::cout << "\t";
            // TODO: change to inline converter
            switch (accessor.type /* accessor.type */) {
            case EcsDataType::INT: std::cout << "si32 "; break;
            case EcsDataType::UINT: std::cout << "ui32 "; break;
            case EcsDataType::FLOAT: std::cout << "f32 "; break;
            case EcsDataType::VEC3: std::cout << "vec3 "; break;
            case EcsDataType::MAT3: std::cout << "mat3 "; break;
            case EcsDataType::MAT4: std::cout << "mat4 "; break;
            case EcsDataType::RESOURCE: std::cout << "ResRef "; break;
            default: break;
            }
            std::cout << q.first << ";" << std::endl;
        }

        // Close struct
        std::cout << "} " << "Component" << componentName << ";\n" << std::endl;
    }

    // Create initializer Component Register
    std::cout << "static inline void\n_ecs_init_internal(ECS *ecs) {" << std::endl;
    for (const auto &p : map) {
        std::string nameUpper = p.first;
        std::for_each(nameUpper.begin(), nameUpper.end(), [](char &c) {
            c = ::toupper(c);
        });
        std::cout << "\tecs_component_register(ecs, C_" << nameUpper
                  << ", sizeof(Component" << p.first << "));" << std::endl;
    }
    std::cout << "}" << std::endl;

    return 1;
}