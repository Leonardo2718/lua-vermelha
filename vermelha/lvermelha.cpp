// vermelha headers
#include "luavm.hpp"

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
   return nullptr;
}
