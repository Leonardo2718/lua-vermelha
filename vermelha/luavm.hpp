#ifndef LUAVM_HPP
#define LUAVM_HPP

#ifdef LUA_C_LINKAGE
extern "C" {
#endif

// Lua headers
#include "lua/lopcodes.h"
#include "lua/lobject.h"
#include "lua/ltm.h"
#include "lua/lstate.h"
#include "lua/ldebug.h"
#include "lua/lvermelha.h"

#ifdef LUA_C_LINKAGE
}
#endif

#endif // LUAVM_HPP

