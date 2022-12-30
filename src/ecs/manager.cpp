#include "manager.hpp"

#include <ecs/lua/eventstore.hpp>

using namespace ecs;

Manager::Manager()
{

    ComponentLayout *t;
    // Component c;

    // m_ComponentList =
}

void
Manager::AddEntity(Entity e)
{
}

void
Manager::ECSEvent(enum ECSEvent event)
{

    // Call the Lua functions as well
    LuaEventStore::ECSEvent(event);
}
