// vermelha headers
#include "lfunctionbuilder.hpp"
#include <stdio.h>

// JitBuilder headers
#include "ilgen/BytecodeBuilder.hpp"

static void printAddr(unsigned char* addr) {
   printf("Compiled! (%p)\n", addr);
}

Lua::FunctionBuilder::FunctionBuilder(Proto* p, Lua::TypeDictionary* types)
   : TR::MethodBuilder(types), prototype(p), luaTypes(types->getLuaTypes()) {

   char file_name_buff[1024];
   snprintf(file_name_buff, 1024, "%s", getstr(prototype->source));

   char line_num_buff[1024];
   snprintf(line_num_buff, 1024, "%d", prototype->linedefined);

   // since Lua functions do not have actual names, use the format
   // `<filename>_<first line of definition>_<last line of definition>`
   // as the "unique" function name
   char func_name_buff[1024];
   snprintf(func_name_buff, 1024, "%s_%s_%d", file_name_buff, line_num_buff, prototype->lastlinedefined);

   DefineLine(line_num_buff);
   DefineFile(file_name_buff);

   DefineName(func_name_buff);
   DefineParameter("L", types->PointerTo("lua_State"));
   DefineReturnType(NoType);
   //setUseBytecodeBuilders();
   
   DefineFunction("printAddr", "0", "0", (void*)printAddr, NoType, 1, Address);
}
    

bool Lua::FunctionBuilder::buildIL() {
   auto instructionCount = prototype->sizecode;
   auto instructions = prototype->code;

   auto L = Load("L");
   auto ci = LoadIndirect("lua_State", "ci", L);
   auto base = LoadIndirect("CallInfo", "u_l_base", ci);

   for (auto i = instructions; i - instructions < instructionCount; ++i) {
      auto arg_a = GETARG_A(*i);
      auto ra = Add(base, ConstInt32(arg_a*(sizeof(TValue))));

      auto opcode = GET_OPCODE(*i);
      if (opcode == OP_LOADK) {
         // rb = k + GETARG_Bx(i);
         auto arg_b = GETARG_Bx(*i);
         auto _rb = prototype->k + arg_b;
         auto rb = ConstInt64(_rb);

         // *ra = *rb;
         auto rb_value = LoadIndirect("TValue", "value_", rb);
         auto rb_tt = LoadIndirect("TValue", "tt_", rb);
         StoreIndirect("TValue", "value_", ra, rb_value);
         StoreIndirect("TValue", "tt_", ra, rb_tt);
      }
      else {
         break;
      }

      // pc++;
      auto pc = LoadIndirect("CallInfo", "u_l_savedpc", ci);
      StoreIndirect("CallInfo", "u_l_savedpc", ci, Add(pc, ConstInt32(1*sizeof(Instruction))));
   }

   StoreIndirect("lua_State", "ci", L, ci);
   Store("L", L);
   Return();
   return true;
}
