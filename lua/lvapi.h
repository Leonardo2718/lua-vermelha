/*
** Helper functions and macros for Lua Vermelha JIT control API
** See Copyright Notice in lua.h
*/

#ifndef lvapi_h
#define lvapi_h

#include "luav.h"

/*
** Generic getters and setters for JIT flags
*/
#define LUAJ_CHECKJITFLAGS(p, f) ((p)->jitflags & f)
#define LUAJ_SETJITFLAGS(p, f) do { \
   Proto* _p = (p); _p->jitflags = _p->jitflags | f; } while (0)
#define LUAJ_CLEARJITFLAGS(p, f) do { \
   Proto* _p = (p); _p->jitflags = _p->jitflags & (~f); } while(0)

/*
** Specific getters and setters for JIT flags
*/
#define LUAJ_ISBLACKLISTED(p) LUAJ_CHECKJITFLAGS(p, LUA_JITBLACKLIST)
#define LUAJ_BLACKLIST(p) LUAJ_SETJITFLAGS(p, LUA_JITBLACKLIST)
#define LUAJ_UNBLACKLIST(p) LUAJ_CLEARJITFLAGS(p, LUA_JITBLACKLIST)

/*
** Check if function has been compiled
*/
#define LUAJ_ISCOMPILED(p) ((p)->compiledcode ? 1 : 0)

#endif /* lvapi_h */
