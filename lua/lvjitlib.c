/*
** Library for controlling the Lua Vermelha JIT
** See Copyright Notice in lua.h
*/

#include "lua.h"
#include "luav.h"

#include "lauxlib.h"
#include "lualib.h"

static int lvjit_checkjitflags(lua_State* L) {
   int flags = lua_tointeger(L, -1);
   int ret = lua_checkjitflags(L, -2, flags);
   lua_pushinteger(L, ret);

   return 1;
}

int lvjit_setjitflags(lua_State* L) {
   int flags = lua_tointeger(L, -1);
   lua_setjitflags(L, -2, flags);
   return 0;
}

int lvjit_clearjitflags(lua_State* L) {
   int flags = lua_tointeger(L, -1);
   lua_clearjitflags(L, -2, flags);
   return 0;
}

static int lvjit_initcallcounter(lua_State* L) {
   int ret = lua_initcallcounter(L);
   lua_pushinteger(L, ret);
   return 1;
}

static int lvjit_iscompiled(lua_State* L) {
   int ret = lua_iscompiled(L, -1);
   lua_pushboolean(L, ret);
   return 1;
}

static int lvjit_compile(lua_State* L) {
   int ret = lua_compile(L, -1);
   lua_pushboolean(L, ret);
   return 1;
}

static const luaL_Reg lvjitlib[] = {
   {"checkjitflags", lvjit_checkjitflags},
   {"setjitflags", lvjit_setjitflags},
   {"clearjitflags", lvjit_clearjitflags},
   {"initcallcounter", lvjit_initcallcounter},
   {"iscompiled", lvjit_iscompiled},
   {"compile", lvjit_compile},
   /* placeholders for JIT flags */
   {"NOJITFLAGS", NULL},
   {"JITBLACKLIST", NULL},
   {NULL, NULL}
};

LUAMOD_API int luaopen_lvjit(lua_State* L) {
   luaL_newlib(L, lvjitlib);
   lua_pushinteger(L, LUA_NOJITFLAGS);
   lua_setfield(L, -2, "NOJITFLAGS");
   lua_pushinteger(L, LUA_JITBLACKLIST);
   lua_setfield(L, -2, "JITBLACKLIST");
   return 1;
}

