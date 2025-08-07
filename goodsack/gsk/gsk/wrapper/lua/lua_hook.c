/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "lua_hook.h"

#include "runtime/gsk_runtime_wrapper.h"

int
gsk_lua_hook_run(const char *event_name, const char *fmt, ...)
{
    va_list ap;
    u64 i, n_args = 0;

    lua_State *L = (lua_State *)gsk_runtime_get_lua_state();
    if (L == NULL) { return 0; }

    // 1) push hook.Run
    lua_getglobal(L, "hook");   // … hook
    lua_getfield(L, -1, "Run"); // … hook, hook.Run
    lua_remove(L, -2);          // … hook.Run

    // 2) first arg: event name
    lua_pushstring(L, event_name);
    n_args = 1;

    // 3) push each arg according to fmt[]
    va_start(ap, fmt);
    for (i = 0; fmt[i] != '\0'; ++i)
    {
        switch (fmt[i])
        {
        case 'i': lua_pushinteger(L, va_arg(ap, int)); break;
        case 'n': lua_pushnumber(L, va_arg(ap, double)); break;
        case 's': lua_pushstring(L, va_arg(ap, const char *)); break;
        case 'u': lua_pushlightuserdata(L, va_arg(ap, void *)); break;
        default:
            LOG_ERROR("unknown fmt '%c'\n", fmt[i]);
            va_end(ap);
            return 0;
        }
        ++n_args;
    }
    va_end(ap);

    // 4) do the call (n_args in, 1 result out)
    if (lua_pcall(L, (int)n_args, 1, 0) != LUA_OK)
    {
        const char *err = lua_tostring(L, -1);
        LOG_ERROR("hook.Run error: %s\n", err);
        lua_pop(L, 1);
        return 0;
    }

    // 5) fetch one integer result (or tweak as needed)
    int ret = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return ret;
}