#include "ecs_handle.hpp"

#include <entity/lua/eventstore.hpp>

entity::ECSHandle::ECSHandle()
{

    ECSComponentLayout *t;
    // Component c;

    // m_ComponentList =
    m_systemFunctionsList = (struct ECSSystemFunction **)malloc(
      sizeof(struct Lua_Functions *) * ECSEVENT_LAST + 1);

    for (int i = 0; i < ECSEVENT_LAST + 1; i++) {
        m_systemFunctionsList[i] = (struct ECSSystemFunction *)malloc(
          sizeof(struct ECSSystemFunction *));
        m_systemFunctionsList[i]->size      = 0;
        m_systemFunctionsList[i]->functions = (int *)calloc(0, sizeof(int));
    }
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

    for (int i = 0; i < m_systemFunctionsList[event]->size; i++) {
        // Call function for every entity
    }

    // Call the Lua functions as well
    LuaEventStore::ECSEvent(event);
}
