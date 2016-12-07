// vermelha headers
#include "luavm.hpp"
#include "LuaTypeDictionary.hpp"
#include "LuaFunctionBuilder.hpp"

// JitBuilder headers
#include "Jit.hpp"

// OMR headers
#include "ilgen/TypeDictionary.hpp"

/*
** JitBuilder functions
*/
int luaJ_initJit() {
   return initializeJit() ? 0 : 1;
}

void luaJ_stopJit() {
   shutdownJit();
}

lua_JitFunction luaJ_compile(Proto* p) {
   Lua::TypeDictionary types;
   uint8_t* entry = nullptr;
   Lua::FunctionBuilder f(p, &types);
   uint32_t rc = compileMethodBuilder(&f, &entry);
   if (rc == 0)
      return (lua_JitFunction)entry;
   else
      return nullptr;
}
