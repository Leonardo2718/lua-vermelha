/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2016, 2017
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

int main() {
   lua_State* L = luaL_newstate();
   assert(100 == lua_initcallcounter(L));
   lua_close(L);
   return 0;
}
