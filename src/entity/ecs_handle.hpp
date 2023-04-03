#ifndef HPP_ECS_HANDLE
#define HPP_ECS_HANDLE

#include <vector>

#include <entity/component/component.hpp>
#include <entity/component/layout.hpp>
#include <entity/v1/ecs.h>

namespace entity {

class ECSHandle {
    ECSHandle();

   public:
    void Initialize();
    void AddEntity(Entity e);
    void ECSEvent(enum ECSEvent event);

   private:
    std::vector<Component *> m_ComponentList;
};

} // namespace entity

#endif // HPP_ECS_HANDLE
