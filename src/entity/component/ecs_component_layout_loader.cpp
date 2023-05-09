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

    std::map<std::string, DataType> dataTypes = {
      // Numeric
      {"int", (DataType {sizeof(int), 1, DataType_E::TYPE_INT})},
      {"float", (DataType {sizeof(float), 1, DataType_E::TYPE_FLOAT})},
      {"bool", (DataType {sizeof(char), 1})},
      // Vector
      {"vec2", (DataType {sizeof(float), 2})},
      {"vec3", (DataType {sizeof(float), 3})},
      {"vec4", (DataType {sizeof(float), 4})},
      // Matrix
      {"mat2", (DataType {sizeof(float[2]), 2})},
      {"mat3", (DataType {sizeof(float[3]), 3})},
      {"mat4", (DataType {sizeof(float[4]), 4})},
      // Resource reference (void ptr)
      {"Resource", (DataType {sizeof(void *), 1})},
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

                data[JData[i]] =
                  (Accessor {0, type.second.size, type.second.stride});
                // std::cout << type.first << ": " << JData[i] << std::endl;
            }
        }

        component->SetData(data);
        layouts[cmp.key()] = component; // i.e, layouts["ComponentTransform"]
    }

    return layouts;
}

int
entity::component::generate_cpp_types(std::string path,
                                      entity::component::ComponentLayoutMap map)
{
    std::cout << "// @generated" << std::endl;
    std::cout << "#include <test.h>\n" << std::endl;
    for (const auto &p : map) {
        // std::cout << p.first << '\t' << p.second << std::endl;

        std::string componentName = p.first;
        // std::cout << "struct " << componentName << " {\n};" << std::endl;
        std::cout << "struct " << componentName << " { " << std::endl;

        ECSComponentLayout *layout = map[componentName];
        for (const auto &q : layout->getData()) {
            Accessor accessor = layout->getData()[q.first];

            // TODO: Get data type
            switch (accessor.position /* accessor.type */) {
            default: break;
            }
            std::cout << "\t float " << q.first << ";" << std::endl;
        }

        std::cout << "};\n" << std::endl;
    }
    return 1;
}