/*
** Lua Vermelha JIT interface
** See Copyright Notice in lua.h
*/

#ifndef lvermelha_h
#define lvermelha_h

#include "lobject.h"

/*
** Initialize the JIT
*/
int luaJ_initJit();

/*
** Shutdown the JIT
*/
void luaJ_stopJit();

/*
** Given a Lua function's `Proto`, tries to compile it. If successful,
** returns a pointer to the compiled function body, `NULL` otherwise.
** This function ignors all JIT flags and does not set `p->compiledcode`.
*/
lua_JitFunction luaJ_invokejit(Proto* p);

/*
** Given a Lua function's `Proto`, tries to compile it. If successful,
** returns 1 and `p->compiledcode` points to the compiled function body.
** Otherwise, returns 0 and `p->compiledcode` is `NULL`.
*/
int luaJ_compile(Proto* p);

/*
** Get the number of times a function must be called before tyring to compile it
*/
unsigned int luaJ_initcallcounter();

#endif /* lvermelha_h */

