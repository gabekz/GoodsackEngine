#ifndef HPP_ECS_HANDLE
#define HPP_ECS_HANDLE

#include <vector>

#include <entity/component/ecs_component.hpp>
#include <entity/component/ecs_component_layout.hpp>
#include <entity/ecsdefs.h>

namespace entity {

class ECSHandle {
    ECSHandle();

   public:
    void Initialize();
    // void AddEntity(Entity e);
    void ECSEvent(enum ECSEvent event);

   private:
    std::vector<ECSComponent *> m_ComponentList;
};

} // namespace entity

#endif // HPP_ECS_HANDLE
