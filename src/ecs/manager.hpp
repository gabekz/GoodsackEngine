#ifndef HPP_MANAGER
#define HPP_MANAGER

#include <vector>

#include <ecs/component/component.hpp>
#include <ecs/component/layout.hpp>
#include <ecs/ecs.h>

namespace ecs {

class Manager {
    Manager();

   public:
    void AddEntity(Entity e);
    void ECSEvent(enum ECSEvent event);

   private:
    std::vector<Component *> m_ComponentList;
};

} // namespace ecs

#endif // HPP_MANAGER
