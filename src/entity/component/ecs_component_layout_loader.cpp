#include "ecs_component_layout_loader.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <util/maths.h>

#include <nlohmann/json.hpp>
#include <stdlib.h>

#include <entity/component/ecs_component.hpp>
#include <entity/component/ecs_component_layout.hpp>

using json = nlohmann::json;
using namespace entity;

std::map<std::string, ECSComponentLayout *>
entity::ParseComponents(std::string path, ui32 rawData)
{
    json JSON;

    if (rawData <= 0) {
        std::ifstream file(path);
        JSON = json::parse(file);
    } else {
        JSON = json::parse(path);
    }
    // std::cout << JSON.items() << std::endl;

    // std::vector<std::string> types = {"vec3", "float", "int"};

    std::map<std::string, DataType> dataTypes;
    dataTypes["int"]   = (DataType {sizeof(int), 1});
    dataTypes["float"] = (DataType {sizeof(float), 1});

    // dataTypes["vec2"]   = (DataType){sizeof(float), 2};
    dataTypes["vec3"] = (DataType {sizeof(float), 3});
    dataTypes["vec4"] = (DataType {sizeof(float), 4});

    // dataTypes["mat3"]   = (DataType){sizeof(vec3) * 4,  3};
    dataTypes["mat4"] = (DataType {sizeof(vec4), 4});
    // strings are handled differently

    // std::vector<ComponentLayout*> layouts;
    std::map<std::string, ECSComponentLayout *> layouts;

    // Loop through every component
    for (auto &cmp : JSON.items()) {
        std::map<std::string, Accessor> data;

        // std::cout << cmp.key() << std::endl;

        ECSComponentLayout *component =
          new ECSComponentLayout(cmp.key().c_str());

        // Every type of available data
        for (auto type : dataTypes) {

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

    /*
    Component *p = new Component(*layouts["ComponentCamera"]);

    vec3 vectorA = {0.25f, 5.8f, 1.0f};
    vec3 asgn = GLM_VEC3_ZERO_INIT;

    p->SetVariable("position", vectorA);
    p->GetVariable("position", &asgn);
    //Accessor acr = layouts["ComponentCamera"]->getAccessor("position");
    //glm_vec3_copy((float *)((char *)p->m_Data.mem+acr.position), asgn);

    printf("\n%f, %f, %f",asgn[0], asgn[1], asgn[2]);
    */

    return layouts;
}
