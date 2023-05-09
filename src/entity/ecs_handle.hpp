#ifndef HPP_ECS_HANDLE
#define HPP_ECS_HANDLE

#include <vector>

#include <entity/component/ecs_component.hpp>
#include <entity/ecsdefs.h>

// typedef void (*ECSSubscriber)(Entity);
typedef void (*ECSSubscriber2)();

namespace entity {

union ECSSystem {
    struct
    {
        ECSSubscriber2 init, destroy, render, update;
    };

    ECSSubscriber2 subscribers[ECSEVENT_LAST + 1];
};

struct ECSSystemFunction
{
    int size;
    int *functions;
};

class ECSHandle {
    ECSHandle();

   public:
    void Initialize();
    // void AddEntity(Entity e);
    void ECSEvent(enum ECSEvent event);

    // Registered Systems
    void RegsterSystem();
    // LuaEventStore();

   protected:
    struct ECSSystemFunction **m_systemFunctionsList;
    std::vector<ECSComponent *> m_ComponentList;
};

class ECSSystemStoreBase {
};

class ECSSystemStoreLua : public ECSSystemStoreBase {
};

} // namespace entity

#endif // HPP_ECS_HANDLE
