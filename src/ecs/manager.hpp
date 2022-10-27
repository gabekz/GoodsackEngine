#ifndef HPP_MANAGER
#define HPP_MANAGER

#include <vector>

#include <ecs/ecs.h>
#include <ecs/component/component.hpp>
#include <ecs/component/layout.hpp>

namespace ecs {

class Manager {
    Manager();

public:
    void AddEntity(Entity e);
    void ECSEvent(enum ECSEvent event);

private:
    std::vector<Component*> m_ComponentList;

};

}

#endif // HPP_MANAGER
