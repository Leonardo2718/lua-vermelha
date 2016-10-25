#ifndef LUAVM_HPP
#define LUAVM_HPP

#ifdef LUA_C_LINKAGE
extern "C" {
#endif

// Lua headers
#include "lua/lopcodes.h"
#include "lua/lobject.h"
#include "lua/lfunc.h"
#include "lua/ltm.h"
#include "lua/lstate.h"
#include "lua/ltable.h"
#include "lua/ldo.h"
#include "lua/lvm.h"
#include "lua/lgc.h"
#include "lua/ldebug.h"
#include "lua/lvermelha.h"

#ifdef LUA_C_LINKAGE
}
#endif

#endif // LUAVM_HPP

