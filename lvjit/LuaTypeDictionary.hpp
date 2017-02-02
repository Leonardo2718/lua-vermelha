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

#ifndef LUATYPEDICTIONARY_HPP
#define LUATYPEDICTIONARY_HPP

// JitBuilder headers
#include "Jit.hpp"
#include "ilgen/TypeDictionary.hpp"

// Lua headers
#include "luavm.hpp"

namespace Lua { class TypeDictionary; }

/*
** A TypeDictionary for defining JitBuilder representations of Lua VM types
*/
class Lua::TypeDictionary : public TR::TypeDictionary {
public:

   // struct for caching JitBuilder representations of commonly used VM types
   struct LuaTypes {
      TR::IlType* lu_byte;
      TR::IlType* lua_Integer;
      TR::IlType* lua_Unsigned;
      TR::IlType* lua_Number;
      TR::IlType* Instruction;
      TR::IlType* Value;
      TR::IlType* TValue;
      TR::IlType* StkId;
      TR::IlType* Proto;
      TR::IlType* LClosure;
      TR::IlType* UpVal;
      TR::IlType* CallInfo;
      TR::IlType* lua_State;
      TR::IlType* TMS;
   };
   
   TypeDictionary();

   LuaTypes getLuaTypes() { return luaTypes; }

private:
   LuaTypes luaTypes;
};

#endif // LUATYPEDICTIONARY_HPP
