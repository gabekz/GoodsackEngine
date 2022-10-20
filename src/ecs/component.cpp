#include "component.hpp"

#include <iostream>
#include <string>
#include <map>
#include <fstream>

#include <stdlib.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace cmptest;

ComponentLayout::ComponentLayout(const char *name) { m_Name = name; }

void ComponentLayout::SetData(std::map<std::string, Accessor> data) {
    ulong sizeReq = 0;
    for(auto& var : data) {
        var.second.position = sizeReq;
        sizeReq += var.second.size;
    }

    m_Variables = data;
    m_Data.mem = malloc((char)sizeReq);
    m_Data.size = sizeReq;
}

void* ComponentLayout::GetVariable(const char *var) {
    if(m_Variables[var].size) {
        //std::cout << m_Variables[var].size << std::endl;
        return (void *)((char *)m_Data.mem+m_Variables[var].position);
    }
    return nullptr;
}

void ComponentLayout::SetVariable(const char *var, void *value) {
    if(m_Variables[var].size) {
        Accessor acr = m_Variables[var];
        memcpy((char *)m_Data.mem+acr.position, value, acr.size * acr.stride);
    }
}

void cmptest::run() {

    std::ifstream file("../res/components.json");
    json JSON = json::parse(file);
    //std::cout << JSON.items() << std::endl;
    
    //std::vector<std::string> types = {"vec3", "float", "int"};

    std::map<std::string, DataType> dataTypes;
    dataTypes["float"] = (DataType){sizeof(float), 1};
    dataTypes["int"] = (DataType){sizeof(int), 1};
    dataTypes["vec3"] = (DataType){sizeof(float), 3};
    // strings are handled differently


    std::vector<ComponentLayout*> componentsList;

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
                    continue;
                }

                data[JData[i]] = (Accessor){0, type.second.size, type.second.stride};
                //std::cout << type.first << ": " << JData[i] << std::endl;
            }
        }

        //std::cout << data["position"].size << std::endl;
        //std::cout << data["rotation"].size << std::endl;
        //std::cout << data["scale"].size << std::endl;

        component->SetData(data);
        componentsList.push_back(component);
    }

    for(auto i : componentsList) {
        std::cout << i->getName() << std::endl;
        int setDtr = 3;
        i->SetVariable("fov", &setDtr);
        int *t = (int *)i->GetVariable("fov");
        if(t)  std::cout << *t << std::endl;
    }
}
