/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2016, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 ******************************************************************************/

#include <assert.h>

extern "C" {
#include "lua.h"
#include "luav.h"
#include "lauxlib.h"
#include "lualib.h"
}

#define  SCRIPT "return 3"

int main() {
   lua_State* L = luaL_newstate();
   luaL_openlibs(L);
   int rc = luaL_loadstring(L, SCRIPT);
   assert(LUA_OK == rc);
   assert(!lua_iscompiled(L, -1));
   lua_compile(L, -1);
   assert(lua_iscompiled(L, -1));
   lua_call(L, 0, 1);
   assert(3 == lua_tointeger(L, -1));
   lua_close(L);
   return 0;
}
