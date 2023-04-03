#include "ecs_handle.hpp"

#include <entity/lua/eventstore.hpp>

entity::ECSHandle::ECSHandle()
{

    ECSComponentLayout *t;
    // Component c;

    // m_ComponentList =
}
void
entity::ECSHandle::Initialize()
{
}

/*
void
entity::ECSHandle::AddEntity(Entity e)
{
}
*/

void
entity::ECSHandle::ECSEvent(enum ECSEvent event)
{

    // Call the Lua functions as well
    LuaEventStore::ECSEvent(event);
}
