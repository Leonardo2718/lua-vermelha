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

   if (rc == 0) {
      return (lua_JitFunction)entry;
   }
   else {
      return nullptr;
   }
}

unsigned int luaJ_initcallcounter() {
   return 100;
}
