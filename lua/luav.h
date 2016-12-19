/*
** Lua Vermelha JIT control API
** See Copyright Notice in lua.h
*/

#ifndef luav_h
#define luav_h

#include "lua.h"

/*
** Function flags for Lua Vermelha JIT
*/
#define LUA_NOJITFLAGS 0x0
#define LUA_JITBLACKLIST  0x1   /* shouldn't (re)compile function */

/*
** Check if the given JIT flags is set for the function at the given index.
*/
LUA_API int lua_checkjitflags(lua_State* L, int index, int flags);

/*
** Set the given JIT flags for the function at the given index.
*/
LUA_API void lua_setjitflags(lua_State* L, int index, int flags);

/*
** Clear the given JIT flags for the function at the given index.
*/
LUA_API void lua_clearjitflags(lua_State* L, int index, int flags);

/*
** Returns the initial value of Lua function call counters.
*/
LUA_API int lua_initcallcounter(lua_State* L);

/*
** Check if the function at the given index has been compiled. Returns 1 if
** true, 0 otherwise.
*/
LUA_API int lua_iscompiled(lua_State* L, int index);

/*
** Compile the Lua function at the given index. Returns 1 if succesfull,
** 0 otherwise.
*/
LUA_API int lua_compile(lua_State* L, int index);

#endif /* luav_h */
