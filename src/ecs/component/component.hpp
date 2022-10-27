#ifndef HPP_COMPONENT
#define HPP_COMPONENT

#include <string>
#include <map>

#include <ecs/component/layout.hpp>

namespace ecs {

class Component {
public:
    Component(ComponentLayout &layout);

    void SetVariable(std::string var, void *value);
    const char* getName() { return m_ComponentLayout.getName(); };

    template <typename T>
    int GetVariable(std::string var, T *destination) {
        Accessor acr = m_ComponentLayout.getAccessor(var);
        if(acr.size) {
            *destination = *(T *)((char *)m_Data.mem+acr.position);
            return 1;
        }
        return 0;
    }

private:
    ComponentLayout &m_ComponentLayout;
    struct { void *mem; int size, index; } m_Data;
};

}


#endif // HPP_COMPONENT
