// vermelha headers
#include "lfunctionbuilder.hpp"
#include <stdio.h>

// JitBuilder headers
#include "ilgen/BytecodeBuilder.hpp"

static void printAddr(unsigned char* addr) {
   printf("Compiled! (%p)\n", addr);
}

static void printBase(unsigned char* addr) {
   printf("base = (%p)\n", addr);
}

static void printRA(unsigned char* addr) {
   printf("ra = (%p)\n", addr);
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
   
   DefineFunction((char*)"printAddr", (char*)"0", (char*)"0", (void*)printAddr, NoType, 1, Address);
   DefineFunction((char*)"printBase", (char*)"0", (char*)"0", (void*)printBase, NoType, 1, Address);
   DefineFunction((char*)"printRA", (char*)"0", (char*)"0", (void*)printRA, NoType, 1, Address);
}
    

bool Lua::FunctionBuilder::buildIL() {
   auto instructionCount = prototype->sizecode;
   auto instructions = prototype->code;

   auto L = Load("L");
   auto ci = LoadIndirect("lua_State", "ci", L);
   auto base = LoadIndirect("CallInfo", "u_l_base", ci);

   for (auto i = instructions; i - instructions < instructionCount; ++i) {
      auto arg_a = GETARG_A(*i);
      auto ra = Add(base, Mul(ConstInt32(arg_a), ConstInt32((sizeof(TValue)))));
      //auto ra = IndexAt(luaTypes.StkId, base, ConstInt32(arg_a)); // IndexAt does not yet support arrays of structs

      auto opcode = GET_OPCODE(*i);
      if (opcode == OP_LOADK) {
         // rb = k + GETARG_Bx(i);
         auto arg_b = GETARG_Bx(*i);
         auto rb = ConstInt64(prototype->k + arg_b);

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
      auto newpc = IndexAt(typeDictionary()->PointerTo(luaTypes.Instruction), pc, ConstInt32(1));
      StoreIndirect("CallInfo", "u_l_savedpc", ci, newpc);
   }

   StoreIndirect("lua_State", "ci", L, ci);
   Store("L", L);
   Return();
   return true;
}
