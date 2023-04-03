#ifndef HPP_MANAGER
#define HPP_MANAGER

#include <vector>

#include <entity/component/component.hpp>
#include <entity/component/layout.hpp>
#include <entity/v1/ecs.h>

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
