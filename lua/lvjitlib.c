/*
** Library for controlling the Lua Vermelha JIT
** See Copyright Notice in lua.h
*/

#include "lua.h"
#include "luav.h"

#include "lauxlib.h"
#include "lualib.h"

static int lvjit_isblacklisted(lua_State* L) {
   int ret = lua_checkjitflags(L, 1, LUA_BLACKLIST);
   lua_pushboolean(L, ret);

   return 1;
}

int lvjit_blacklist(lua_State* L) {
   lua_setjitflags(L, 1, LUA_BLACKLIST);
   return 0;
}

int lvjit_clearblacklist(lua_State* L) {
   lua_clearjitflags(L, 1, LUA_BLACKLIST);
   return 0;
}

static int lvjit_initcallcounter(lua_State* L) {
   int ret = lua_initcallcounter(L);
   lua_pushinteger(L, ret);
   return 1;
}

static int lvjit_iscompiled(lua_State* L) {
   int ret = lua_iscompiled(L, 1);
   lua_pushboolean(L, ret);
   return 1;
}

static int lvjit_compile(lua_State* L) {
   int ret = lua_compile(L, 1);
   lua_pushboolean(L, ret);
   return 1;
}

static const luaL_Reg lvjitlib[] = {
   {"isblacklisted", lvjit_isblacklisted},
   {"blacklist", lvjit_blacklist},
   {"clearblacklist", lvjit_clearblacklist},
   {"initcallcounter", lvjit_initcallcounter},
   {"iscompiled", lvjit_iscompiled},
   {"compile", lvjit_compile},
   {NULL, NULL}
};

LUAMOD_API int luaopen_lvjit(lua_State* L) {
   luaL_newlib(L, lvjitlib);
   return 1;
}

