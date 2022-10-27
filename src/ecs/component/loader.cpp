#include "loader.hpp"

#include <iostream>
#include <string>
#include <map>
#include <fstream>

#include <stdlib.h>
#include <nlohmann/json.hpp>

#include <ecs/component/component.hpp>
#include <ecs/component/layout.hpp>

using json = nlohmann::json;
using namespace ecs;

std::map<std::string, ComponentLayout*> ecs::ParseComponents(const char *path) {

    std::ifstream file(path);
    json JSON = json::parse(file);
    //std::cout << JSON.items() << std::endl;
    
    //std::vector<std::string> types = {"vec3", "float", "int"};

    std::map<std::string, DataType> dataTypes;
    dataTypes["float"]  = (DataType){sizeof(float), 1};
    dataTypes["vec3"]   = (DataType){sizeof(float), 3};
    dataTypes["int"]    = (DataType){sizeof(int),   1};
    // strings are handled differently


    //std::vector<ComponentLayout*> layouts;
    std::map<std::string, ComponentLayout*> layouts;

    // Loop through every component
    for(auto& cmp : JSON.items()) {
        std::map<std::string, Accessor> data;

        std::cout << cmp.key() << std::endl;

        ComponentLayout *component = new ComponentLayout(cmp.key().c_str());

        // Every type of available data
        for(auto type : dataTypes) {

            json JData = JSON[cmp.key()][type.first];
            // Every variable of 'type' in the component
            for(int i = 0; i < JData.size(); i++) {

                if(!strcmp(type.first.c_str(), "string")) {
                    // TODO: handle
                    //data[JData[i]] = (Accessor){0, type.second.size, type.second.stride};
                    continue;
                }

                data[JData[i]] = (Accessor){
                    0, type.second.size, type.second.stride };
                //std::cout << type.first << ": " << JData[i] << std::endl;
            }
        }

        component->SetData(data);
        layouts[cmp.key()] = component; // i.e, layouts["ComponentTransform"]
    }

    Component *p = new Component(*layouts["ComponentCamera"]);

    int value, newSpeed = 69;
    p->SetVariable("speed", &newSpeed);
    if(p->GetVariable("speed", &value)) {
        std::cout << value << std::endl;
    }

    return layouts;
}
