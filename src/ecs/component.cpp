#include "component.hpp"

#include <iostream>
#include <string>
#include <map>
#include <fstream>

#include <stdlib.h>

#include <util/maths.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace ecs;

ComponentLayout::ComponentLayout(const char *name) { m_Name = name; }

ComponentLayout::~ComponentLayout() {
    free(m_DataArray);
    free(m_Data.mem);
}

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

template <typename T>
int ComponentLayout::GetVariable(const char *var, T *destination) {
    if(m_Variables[var].size) {
        //std::cout << m_Variables[var].size << std::endl;
        *destination = *(T *)((char *)m_Data.mem+m_Variables[var].position);
        return 1;
    }
    return 0;
}

void ComponentLayout::SetVariable(const char *var, void *value) {
    if(m_Variables[var].size) {
        Accessor acr = m_Variables[var];
        memcpy((char *)m_Data.mem+acr.position, value, acr.size * acr.stride);
    }
}

void ecs::ParseComponents(const char *path) {

    std::ifstream file(path);
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
                    //data[JData[i]] = (Accessor){0, type.second.size, type.second.stride};
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
        setDtr = 5;
        i->SetVariable("speed", &setDtr);

        int t;
        if(i->GetVariable("speed", &t)) {
            std::cout << t << std::endl;
        }

        int z;
        if(i->GetVariable("fov", &z)) {
            std::cout << z << std::endl;
        } 
        vec3 v = {2, 9, 1};
        i->SetVariable("position", &v);
        if(i->GetVariable("position", v)) {
            std::cout << v[0] << v[1] << v[2] << std::endl;
        }

        int test = 15;
        i->SetVariable("speed", &test);
        if(i->GetVariable("speed", &test)) {
            std::cout << " template: " << test << std::endl;

        }
    }
}
