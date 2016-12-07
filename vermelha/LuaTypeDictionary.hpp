#ifndef LBUILDER_HPP
#define LBUILDER_HPP

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
      TR::IlType* TValue;
      TR::IlType* StkId;
      TR::IlType* LClosure;
      TR::IlType* UpVal;
      TR::IlType* CallInfo;
      TR::IlType* lua_State;
   };
   
   TypeDictionary();

   LuaTypes getLuaTypes() { return luaTypes; }

private:
   LuaTypes luaTypes;
};

#endif // LBUILDER_HPP
