/*
** Lua Vermelha JIT control API
** See Copyright Notice in lua.h
*/

#include "lvapi.h"
#include "lvermelha.h"

int lua_compile(Proto* p) {
   LUA_BLACKLIST(p);
   lua_JitFunction f = luaJ_compile(p);

   if (f) {
      p->compiledcode = f;
      return 1;
   }
   else {
      p->compiledcode = NULL;
      return 0;
   }
}

