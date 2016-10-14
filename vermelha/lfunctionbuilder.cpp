#include "lfunctionbuilder.hpp"
#include <stdio.h>

static void printAddr(unsigned char* addr) {
   printf("Compiled! (%p)\n", addr);
}

Lua::FunctionBuilder::FunctionBuilder(Proto* p, Lua::TypeDictionary* types)
   : TR::MethodBuilder(types), prototype(p), luaTypes(types->getLuaTypes()) {

   char file_name_buff[1024];
   snprintf(file_name_buff, 1024, "%s", getstr(prototype->source));

   char line_num_buff[1024];
   snprintf(line_num_buff, 1024, "%d", prototype->linedefined);

   DefineLine(line_num_buff);
   DefineFile(file_name_buff);

   DefineName("FUNCTION");    // since Lua functions do not have names, we provide a default name
   DefineParameter("L", types->PointerTo("lua_State"));
   DefineReturnType(NoType);
   setUseBytecodeBuilders();
   
   DefineFunction("printAddr", "0", "0", (void*)printAddr, NoType, 1, Address);
}
    

bool Lua::FunctionBuilder::buildIL() {
   auto L = Load("L");
   auto ci = LoadIndirect("lua_State", "ci", L);
   auto pc = LoadIndirect("CallInfo", "u_l_savedpc", ci);
   Call("printAddr", 1, pc);
   StoreIndirect("CallInfo", "u_l_savedpc", ci, pc);
   StoreIndirect("lua_State", "ci", L, ci);
   Store("L", L);
   Return();
   return true;
}
