/*
** Lua Vermelha JIT control API
** See Copyright Notice in lua.h
*/

#include "luav.h"
#include "lvapi.h"
#include "lvermelha.h"

static Proto* getfuncproto(lua_State* L, int index) {
   const void* f = lua_topointer(L, index);
   return ((const LClosure*)f)->p;
}

LUA_API int lua_checkjitflags(lua_State* L, int index, int flags) {
   int ret = (int)LUA_NOJITFLAGS;
   Proto* p = NULL;
   lua_lock(L);
   p = getfuncproto(L, index);
   ret = LUAJ_CHECKJITFLAGS(p, flags);
   lua_unlock(L);
   return ret;
}

LUA_API void lua_setjitflags(lua_State* L, int index, int flags) {
   lua_lock(L);
   Proto* p = getfuncproto(L, index);
   LUAJ_SETJITFLAGS(p, flags);
   lua_unlock(L);
}

LUA_API void lua_clearjitflags(lua_State* L, int index, int flags) {
   lua_lock(L);
   Proto* p = getfuncproto(L, index);
   LUAJ_CLEARJITFLAGS(p, flags);
   lua_unlock(L);
}

LUA_API int lua_initcallcounter(lua_State* L) {
   (void)L; // not used
   return luaJ_initcallcounter();
}

LUA_API int lua_iscompiled(lua_State* L, int index) {
   int ret = 0;
   Proto* p = NULL;
   lua_lock(L);
   p = getfuncproto(L, index);
   ret = LUAJ_ISCOMPILED(p);
   lua_unlock(L);
   return ret;
}

LUA_API int lua_compile(lua_State* L, int index) {
   int ret = 0;
   Proto* p = NULL;
   lua_lock(L);
   p = getfuncproto(L, index);
   ret = luaJ_compile(p);
   lua_unlock(L);
   return ret;
}

