#ifndef HPP_COMPONENT
#define HPP_COMPONENT

#include <map>
#include <string>
#include <type_traits>

#include <entity/component/layout.hpp>
#include <util/maths.h>

namespace ecs {

class Component {
   public:
    Component(ComponentLayout &layout);

    const char *getName() { return m_ComponentLayout.getName(); };
    int SetVariable(std::string var, void *value);

    template <typename T>
    int GetVariable(std::string var, T *destination);

   private:
    struct
    {
        void *mem;
        int size, index;
    } m_Data;
    ComponentLayout &m_ComponentLayout;
};

} // namespace ecs

template <typename T>
int
ecs::Component::GetVariable(std::string var, T *destination)
{
    Accessor acr = m_ComponentLayout.getAccessor(var);
    if (acr.size) {
        // printf("TEST T: %f", *(T *)((char *)m_Data.mem+acr.position));
        *destination = *(T *)((char *)m_Data.mem + acr.position);
        return 1;
    }
    return 0;
}

template <>
inline // vec3 specialization
  int
  ecs::Component::GetVariable<float[3]>(std::string var,
                                        float (*destination)[3])
{
    Accessor acr = m_ComponentLayout.getAccessor(var);
    if (acr.size) {
        glm_vec3_copy((float *)((char *)m_Data.mem + acr.position),
                      *destination);
        return 1;
    }
    return 0;
}

template <>
inline // mat4 specialization
  int
  ecs::Component::GetVariable<float[4][4]>(std::string var,
                                           float (*destination)[4][4])
{
    Accessor acr = m_ComponentLayout.getAccessor(var);
    if (acr.size) {
        glm_mat4_copy((vec4 *)((char *)m_Data.mem + acr.position),
                      *destination);
        return 1;
    }
    return 0;
}

#endif // HPP_COMPONENT
