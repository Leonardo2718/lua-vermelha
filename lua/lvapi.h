/*
** Lua Vermelha JIT control API
** See Copyright Notice in lua.h
*/

#ifndef lvapi_h
#define lvapi_h

#include "lobject.h"

/*
** Function flags for Lua Vermelha JIT
*/
#define LUA_NOJITFLAGS 0x0
#define LUA_BLACK_LIST 0x1   /* shouldn't (re)compile function */

/*
** Getters and setters for JIT flags
*/
#define LUA_CHECKJITFLAG(p, f) ((p)->jitflags & f)
#define LUA_SETJITFLAG(p, f) do { \
   Proto* _p = (p); _p->jitflags = _p->jitflags | f; } while (0)
#define LUA_UNSETJITFLAG(p, f) do { \
   Proto* _p = (p); _p->jitflags = _p->jitflags & (~f); } while(0)

#define LUA_ISBLACKLISTED(p) LUA_CHECKJITFLAG(p, LUA_BLACK_LIST)
#define LUA_BLACKLIST(p) LUA_SETJITFLAG(p, LUA_BLACK_LIST)
#define LUA_UNBLACKLIST(p) LUA_UNSETJITFLAG(p, LUA_BLACK_LIST)

/*
** Get the initial call counter value. This represents the number of times
** a function must be called before it gets JIT compiled
*/
#define LUA_INITCALLCOUNTER luaJ_initcallcounter()

/*
** Check if function has been compiled
*/
#define LUA_ISCOMPILED(p) ((p)->compiledcode ? 1 : 0)

/*
** Given a Lua function's `Proto`, tries to compile it. If successful,
** returns 1 and `p->compiledcode` points to the compiled function body.
** Otherwise, returns 0 and `p->compiledcode` is `NULL`.
** Unconditionally sets the LUAV_BLACK_LIST flag.
*/
int lua_compile(Proto* p);

#endif /* lvapi_h */
