/*
** Lua Vermelha JIT interface
** See Copyright Notice in lua.h
*/

#ifndef lvermelha_h
#define lvermelha_h

#include "lua.h"
#include "lobject.h"

/*
** JitBuilder functions
*/
int luaJ_initJit();
void luaJ_stopJit();

/*
** Given a Lua function's `Proto`, tries to compile it. If successful,
** returns a pointer to the compiled function body, `NULL` otherwise.
*/
lua_JitFunction luaJ_compile(Proto* p);

/*
** Get the number of times a function must be called before tyring to compile it
*/
unsigned int luaJ_initcallcounter();

#endif /* lvermelha_h */

